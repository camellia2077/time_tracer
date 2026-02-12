import json
import os
import subprocess
from pathlib import Path
from typing import Dict, List, Tuple
from urllib.parse import unquote, urlparse


def path_to_uri(path: Path) -> str:
    return path.resolve().as_uri()


def uri_to_path(uri: str) -> Path:
    parsed = urlparse(uri)
    if parsed.scheme != "file":
        raise ValueError(f"Unsupported URI scheme in {uri}")
    path_str = unquote(parsed.path)
    if os.name == "nt" and path_str.startswith("/"):
        path_str = path_str[1:]
    return Path(path_str)


def _utf16_character_to_index(text: str, utf16_character: int) -> int:
    if utf16_character <= 0:
        return 0
    units = 0
    for index, char in enumerate(text):
        if units >= utf16_character:
            return index
        units += 2 if ord(char) > 0xFFFF else 1
    return len(text)


def _line_starts(text: str) -> List[int]:
    starts = [0]
    for index, char in enumerate(text):
        if char == "\n":
            starts.append(index + 1)
    return starts


def _position_to_offset(text: str, line: int, character: int) -> int:
    starts = _line_starts(text)
    if line < 0:
        return 0
    if line >= len(starts):
        return len(text)

    start = starts[line]
    end = starts[line + 1] if line + 1 < len(starts) else len(text)
    line_text = text[start:end]
    char_index = _utf16_character_to_index(line_text, character)
    return min(start + char_index, end)


def _extract_text_edits(workspace_edit: Dict) -> Dict[str, List[Dict]]:
    edits_by_uri: Dict[str, List[Dict]] = {}

    for uri, edits in workspace_edit.get("changes", {}).items():
        edits_by_uri.setdefault(uri, []).extend(edits)

    for change in workspace_edit.get("documentChanges", []):
        if "textDocument" in change and "edits" in change:
            uri = change["textDocument"].get("uri", "")
            if uri:
                edits_by_uri.setdefault(uri, []).extend(change["edits"])

    return edits_by_uri


def count_workspace_edit_edits(workspace_edit: Dict) -> int:
    edits_by_uri = _extract_text_edits(workspace_edit)
    return sum(len(edits) for edits in edits_by_uri.values())


def _is_path_in_roots(path: Path, roots: List[Path]) -> bool:
    path_resolved = path.resolve()
    for root in roots:
        try:
            path_resolved.relative_to(root.resolve())
            return True
        except Exception:
            continue
    return False


class ClangdClient:
    def __init__(
        self,
        clangd_path: str,
        compile_commands_dir: Path,
        root_dir: Path,
        background_index: bool = True,
    ):
        self.clangd_path = clangd_path
        self.compile_commands_dir = compile_commands_dir
        self.root_dir = root_dir
        self.background_index = background_index
        self.process: subprocess.Popen | None = None
        self._next_id = 1
        self._open_docs: Dict[str, Dict] = {}

    def start(self):
        if self.process is not None:
            return
        cmd = [
            self.clangd_path,
            f"--compile-commands-dir={self.compile_commands_dir}",
            "--header-insertion=never",
            "--pch-storage=memory",
        ]
        if self.background_index:
            cmd.append("--background-index")
        else:
            cmd.append("--background-index=false")
        self.process = subprocess.Popen(
            cmd,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
        )
        self._initialize()

    def stop(self):
        if self.process is None:
            return
        try:
            self._request("shutdown", {})
            self._notify("exit", {})
        finally:
            if self.process.poll() is None:
                self.process.terminate()
            self.process = None

    def _initialize(self):
        response = self._request(
            "initialize",
            {
                "processId": os.getpid(),
                "rootUri": path_to_uri(self.root_dir),
                "capabilities": {
                    "workspace": {"workspaceEdit": {"documentChanges": True}},
                    "textDocument": {"rename": {"dynamicRegistration": False}},
                },
            },
        )
        if "error" in response:
            raise RuntimeError(f"clangd initialize failed: {response['error']}")
        self._notify("initialized", {})

    def _next_request_id(self) -> int:
        request_id = self._next_id
        self._next_id += 1
        return request_id

    def _send_message(self, payload: Dict):
        if self.process is None or self.process.stdin is None:
            raise RuntimeError("clangd process is not running")
        encoded = json.dumps(payload, ensure_ascii=False).encode("utf-8")
        header = f"Content-Length: {len(encoded)}\r\n\r\n".encode("ascii")
        self.process.stdin.write(header + encoded)
        self.process.stdin.flush()

    def _read_message(self) -> Dict:
        if self.process is None or self.process.stdout is None:
            raise RuntimeError("clangd process is not running")

        headers = {}
        while True:
            line = self.process.stdout.readline()
            if not line:
                raise RuntimeError("clangd closed the output stream")
            decoded = line.decode("ascii", errors="replace").strip()
            if decoded == "":
                break
            if ":" in decoded:
                key, value = decoded.split(":", 1)
                headers[key.strip().lower()] = value.strip()

        content_length = int(headers.get("content-length", "0"))
        body = self.process.stdout.read(content_length)
        if not body:
            raise RuntimeError("clangd returned an empty message body")
        return json.loads(body.decode("utf-8"))

    def _request(self, method: str, params: Dict) -> Dict:
        request_id = self._next_request_id()
        self._send_message(
            {"jsonrpc": "2.0", "id": request_id, "method": method, "params": params}
        )

        while True:
            message = self._read_message()
            if "id" not in message:
                continue
            if message.get("id") == request_id:
                return message

    def _notify(self, method: str, params: Dict):
        self._send_message({"jsonrpc": "2.0", "method": method, "params": params})

    def _language_id(self, path: Path) -> str:
        suffix = path.suffix.lower()
        if suffix in {".h", ".hpp", ".hh", ".hxx", ".c", ".cc", ".cpp", ".cxx"}:
            return "cpp"
        return "plaintext"

    def _open_document(self, path: Path) -> str:
        resolved = path.resolve()
        uri = path_to_uri(resolved)
        if uri in self._open_docs:
            return uri

        text = resolved.read_text(encoding="utf-8", errors="replace")
        self._open_docs[uri] = {"path": resolved, "text": text, "version": 1}
        self._notify(
            "textDocument/didOpen",
            {
                "textDocument": {
                    "uri": uri,
                    "languageId": self._language_id(resolved),
                    "version": 1,
                    "text": text,
                }
            },
        )
        return uri

    def _notify_document_change(self, uri: str, new_text: str):
        doc = self._open_docs[uri]
        doc["version"] += 1
        doc["text"] = new_text
        self._notify(
            "textDocument/didChange",
            {
                "textDocument": {"uri": uri, "version": doc["version"]},
                "contentChanges": [{"text": new_text}],
            },
        )

    def _apply_text_edits_to_content(self, text: str, edits: List[Dict]) -> str:
        normalized_edits: List[Tuple[int, int, str]] = []
        for edit in edits:
            if "range" not in edit:
                continue
            start = edit["range"]["start"]
            end = edit["range"]["end"]
            start_offset = _position_to_offset(
                text, int(start.get("line", 0)), int(start.get("character", 0))
            )
            end_offset = _position_to_offset(
                text, int(end.get("line", 0)), int(end.get("character", 0))
            )
            normalized_edits.append((start_offset, end_offset, edit.get("newText", "")))

        normalized_edits.sort(key=lambda item: (item[0], item[1]), reverse=True)
        updated = text
        for start_offset, end_offset, new_text in normalized_edits:
            updated = updated[:start_offset] + new_text + updated[end_offset:]
        return updated

    def apply_workspace_edit(
        self,
        workspace_edit: Dict,
        dry_run: bool = False,
        allowed_roots: List[Path] | None = None,
        enforce_all_in_scope: bool = True,
    ) -> Dict[str, List[Path]]:
        edits_by_uri = _extract_text_edits(workspace_edit)
        changed_files: List[Path] = []
        blocked_files: List[Path] = []

        roots = [root.resolve() for root in (allowed_roots or [])]
        if roots:
            for uri in edits_by_uri.keys():
                path = uri_to_path(uri).resolve()
                if not _is_path_in_roots(path, roots):
                    blocked_files.append(path)

            if blocked_files and enforce_all_in_scope:
                return {"changed_files": [], "blocked_files": blocked_files}

        for uri, edits in edits_by_uri.items():
            path = uri_to_path(uri)
            resolved = path.resolve()
            if roots and not _is_path_in_roots(resolved, roots):
                continue
            open_uri = self._open_document(resolved)
            original_text = self._open_docs[open_uri]["text"]
            updated_text = self._apply_text_edits_to_content(original_text, edits)

            if updated_text == original_text:
                continue

            if not dry_run:
                resolved.write_text(updated_text, encoding="utf-8")
                self._notify_document_change(open_uri, updated_text)
            changed_files.append(resolved)

        return {"changed_files": changed_files, "blocked_files": blocked_files}

    def rename_symbol(
        self,
        file_path: Path,
        line: int,
        col: int,
        new_name: str,
        dry_run: bool = False,
        allowed_roots: List[Path] | None = None,
    ) -> Dict:
        resolved = file_path.resolve()
        uri = self._open_document(resolved)
        response = self._request(
            "textDocument/rename",
            {
                "textDocument": {"uri": uri},
                "position": {"line": max(0, line - 1), "character": max(0, col - 1)},
                "newName": new_name,
            },
        )

        if "error" in response:
            return {"ok": False, "error": response["error"]}

        workspace_edit = response.get("result")
        if not workspace_edit:
            return {"ok": False, "error": "No workspace edit returned"}

        edit_count = count_workspace_edit_edits(workspace_edit)
        apply_result = self.apply_workspace_edit(
            workspace_edit,
            dry_run=dry_run,
            allowed_roots=allowed_roots,
            enforce_all_in_scope=True,
        )
        return {
            "ok": True,
            "edit_count": edit_count,
            "changed_files": [str(path) for path in apply_result["changed_files"]],
            "blocked_files": [str(path) for path in apply_result["blocked_files"]],
        }
