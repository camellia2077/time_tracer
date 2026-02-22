from __future__ import annotations

import json
import os
import subprocess
from pathlib import Path

from .codec import path_to_uri, uri_to_path
from .workspace_edit import (
    apply_text_edits_to_content,
    count_workspace_edit_edits,
    extract_text_edits,
    is_path_in_roots,
)


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
        self._open_docs: dict[str, dict] = {}

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

    def _send_message(self, payload: dict):
        if self.process is None or self.process.stdin is None:
            raise RuntimeError("clangd process is not running")
        encoded = json.dumps(payload, ensure_ascii=False).encode("utf-8")
        header = f"Content-Length: {len(encoded)}\r\n\r\n".encode("ascii")
        self.process.stdin.write(header + encoded)
        self.process.stdin.flush()

    def _read_message(self) -> dict:
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

    def _request(self, method: str, params: dict) -> dict:
        request_id = self._next_request_id()
        self._send_message({"jsonrpc": "2.0", "id": request_id, "method": method, "params": params})

        while True:
            message = self._read_message()
            if "id" not in message:
                continue
            if message.get("id") == request_id:
                return message

    def _notify(self, method: str, params: dict):
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

    def apply_workspace_edit(
        self,
        workspace_edit: dict,
        dry_run: bool = False,
        allowed_roots: list[Path] | None = None,
        enforce_all_in_scope: bool = True,
    ) -> dict[str, list[Path]]:
        edits_by_uri = extract_text_edits(workspace_edit)
        changed_files: list[Path] = []
        blocked_files: list[Path] = []

        roots = [root.resolve() for root in (allowed_roots or [])]
        if roots:
            for uri in edits_by_uri.keys():
                path = uri_to_path(uri).resolve()
                if not is_path_in_roots(path, roots):
                    blocked_files.append(path)

            if blocked_files and enforce_all_in_scope:
                return {"changed_files": [], "blocked_files": blocked_files}

        for uri, edits in edits_by_uri.items():
            path = uri_to_path(uri)
            resolved = path.resolve()
            if roots and not is_path_in_roots(resolved, roots):
                continue
            open_uri = self._open_document(resolved)
            original_text = self._open_docs[open_uri]["text"]
            updated_text = apply_text_edits_to_content(original_text, edits)

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
        allowed_roots: list[Path] | None = None,
    ) -> dict:
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
