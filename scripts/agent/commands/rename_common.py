import json
import re
import time
from pathlib import Path
from typing import Dict, List, Optional

from ..services.clangd_lsp import ClangdClient
from .build import BuildCommand


class RenameCommonMixin:
    def _paths(self, app_name: str) -> Dict[str, Path]:
        app_dir = self.ctx.get_app_dir(app_name)
        build_tidy_dir = app_dir / "build_tidy"
        tasks_dir = build_tidy_dir / "tasks"
        rename_dir = build_tidy_dir / "rename"
        return {
            "app_dir": app_dir,
            "build_tidy_dir": build_tidy_dir,
            "tasks_dir": tasks_dir,
            "rename_dir": rename_dir,
            "candidates_path": rename_dir / "rename_candidates.json",
            "plan_md_path": rename_dir / "rename_plan.md",
            "apply_report_json_path": rename_dir / "rename_apply_report.json",
            "apply_report_md_path": rename_dir / "rename_apply_report.md",
            "audit_report_json_path": rename_dir / "rename_audit_report.json",
            "audit_report_md_path": rename_dir / "rename_audit_report.md",
        }

    def _load_candidates(self, candidates_path: Path) -> List[Dict]:
        if not candidates_path.exists():
            return []
        data = json.loads(candidates_path.read_text(encoding="utf-8"))
        return data.get("candidates", [])

    def _is_within_app_scope(self, file_path: Path, app_dir: Path) -> bool:
        try:
            file_path.resolve().relative_to(app_dir.resolve())
            return True
        except Exception:
            return False

    def _resolve_file_path(self, file_path: str, app_dir: Path) -> Path:
        candidate_path = Path(file_path)
        if candidate_path.is_absolute():
            return candidate_path
        return app_dir / candidate_path

    def _resolve_position(
        self, file_path: Path, line: int, col: int, old_name: str
    ) -> tuple[int, int]:
        text = file_path.read_text(encoding="utf-8", errors="replace")
        lines = text.splitlines()
        if line <= 0 or line > len(lines):
            return line, col

        source_line = lines[line - 1]
        expected_col = max(1, col)
        expected_index = expected_col - 1
        if (
            old_name
            and expected_index < len(source_line)
            and source_line[expected_index : expected_index + len(old_name)] == old_name
        ):
            return line, expected_col

        if old_name:
            fallback_index = source_line.find(old_name)
            if fallback_index >= 0:
                return line, fallback_index + 1

            full_text_match = re.search(
                rf"\b{re.escape(old_name)}\b", text, flags=re.MULTILINE
            )
            if full_text_match:
                prior_text = text[: full_text_match.start()]
                fallback_line = prior_text.count("\n") + 1
                line_start = prior_text.rfind("\n")
                fallback_col = (
                    full_text_match.start() + 1
                    if line_start < 0
                    else (full_text_match.start() - line_start)
                )
                return fallback_line, fallback_col

        return line, expected_col

    def _is_header_file(self, file_path: Path) -> bool:
        return file_path.suffix.lower() in {".h", ".hpp", ".hh", ".hxx"}

    def _is_risky_symbol_kind(self, symbol_kind: str) -> bool:
        normalized_kind = symbol_kind.strip().lower()
        return normalized_kind in {
            "function",
            "method",
            "member",
            "class member",
            "private member",
            "protected member",
        }

    def _should_skip_partial_header_rename(
        self,
        symbol_kind: str,
        source_file: Path,
        edit_count: int,
        changed_files: List[str],
    ) -> bool:
        if not self.ctx.config.rename.skip_header_single_edit:
            return False

        if edit_count > 1:
            return False

        if not self._is_header_file(source_file):
            return False

        if not self._is_risky_symbol_kind(symbol_kind):
            return False

        if not changed_files:
            return True

        for changed in changed_files:
            if not self._is_header_file(Path(changed)):
                return False
        return True

    def _count_symbol_occurrences(self, text: str, symbol_name: str) -> int:
        if not symbol_name:
            return 0
        return len(re.findall(rf"\b{re.escape(symbol_name)}\b", text))

    def _count_symbol_in_sibling_sources(
        self,
        header_path: Path,
        old_name: str,
        new_name: str,
    ) -> tuple[int, int]:
        old_count = 0
        new_count = 0
        for ext in (".cpp", ".cc", ".cxx"):
            sibling_path = header_path.with_suffix(ext)
            if not sibling_path.exists():
                continue
            sibling_text = sibling_path.read_text(encoding="utf-8", errors="replace")
            old_count += self._count_symbol_occurrences(sibling_text, old_name)
            new_count += self._count_symbol_occurrences(sibling_text, new_name)
        return old_count, new_count

    def _ensure_tidy_build_ready(self, app_name: str) -> int:
        build_tidy_dir = self.ctx.get_app_dir(app_name) / "build_tidy"
        compile_commands_path = build_tidy_dir / "compile_commands.json"
        if compile_commands_path.exists():
            return 0

        print(f"--- Missing {compile_commands_path}. Running tidy configure...")
        builder = BuildCommand(self.ctx)
        return builder.configure(app_name, tidy=True)

    def _extract_candidate_fields(
        self, candidate: Dict, app_dir: Path
    ) -> tuple[Path, str, str, str, int, int]:
        file_path = self._resolve_file_path(candidate.get("file", ""), app_dir)
        old_name = candidate.get("old_name", "")
        new_name = candidate.get("new_name", "")
        symbol_kind = candidate.get("symbol_kind", "")
        line = int(candidate.get("line", 0))
        col = int(candidate.get("col", 0))
        return file_path, old_name, new_name, symbol_kind, line, col

    def _build_apply_result(
        self,
        index: int,
        status: str,
        reason: str,
        file_path: Path,
        line: int,
        col: int,
        old_name: str,
        new_name: str,
        edit_count: int = 0,
        changed_files: Optional[List[str]] = None,
    ) -> Dict:
        return {
            "id": index,
            "status": status,
            "reason": reason,
            "file": str(file_path),
            "line": line,
            "col": col,
            "old_name": old_name,
            "new_name": new_name,
            "edit_count": edit_count,
            "changed_files": changed_files or [],
        }

    def _start_clangd(self, clangd_client: ClangdClient) -> bool:
        try:
            clangd_client.start()
            return True
        except Exception as exc:
            print(f"--- Failed to start clangd: {exc}")
            return False

    def _wait_for_clangd_warmup(self) -> None:
        warmup_seconds = max(0.0, float(self.ctx.config.rename.clangd_warmup_seconds))
        if warmup_seconds > 0:
            print(f"--- Waiting {warmup_seconds:.1f}s for clangd index warm-up...")
            time.sleep(warmup_seconds)
