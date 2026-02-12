import json
import re
import time
from pathlib import Path
from typing import Dict, List, Optional

from ..core.context import Context
from ..services import rename_planner
from ..services.clangd_lsp import ClangdClient
from .build import BuildCommand
from .tidy import TidyCommand


class RenameCommand:
    def __init__(self, ctx: Context):
        self.ctx = ctx

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

    def plan(
        self,
        app_name: str,
        max_candidates: Optional[int] = None,
        run_tidy: bool = False,
    ) -> int:
        paths = self._paths(app_name)

        if run_tidy:
            print("--- Running tidy before rename plan generation...")
            ret = TidyCommand(self.ctx).execute(app_name, [])
            if ret != 0:
                return ret

        tasks_dir = paths["tasks_dir"]
        if not tasks_dir.exists():
            print(f"--- Tasks directory does not exist: {tasks_dir}")
            print("--- Run `python scripts/run.py tidy --app <app>` first.")
            return 1

        check_name = self.ctx.config.rename.check_name
        all_candidates = rename_planner.collect_rename_candidates(
            tasks_dir=tasks_dir,
            check_name=check_name,
            allowed_kinds=self.ctx.config.rename.allowed_kinds,
            app_root=paths["app_dir"],
        )
        effective_limit = (
            max_candidates
            if max_candidates is not None
            else self.ctx.config.rename.max_candidates_per_run
        )
        outputs = rename_planner.write_plan_outputs(
            rename_dir=paths["rename_dir"],
            all_candidates=all_candidates,
            check_name=check_name,
            max_candidates=effective_limit,
        )

        print(
            "--- Rename plan generated: "
            f"{len(outputs['selected_candidates'])}/{outputs['total_candidates']} candidates"
        )
        print(f"--- JSON: {outputs['json_path']}")
        print(f"--- Markdown: {outputs['md_path']}")
        if outputs["truncated"]:
            print("--- Plan is truncated by max candidate limit.")
        return 0

    def apply(
        self,
        app_name: str,
        limit: int = 0,
        dry_run: bool = False,
        strict: bool = False,
    ) -> int:
        paths = self._paths(app_name)
        candidates = self._load_candidates(paths["candidates_path"])
        if not candidates:
            print("--- No rename candidates found.")
            print("--- Run `python scripts/run.py rename-plan --app <app>` first.")
            return 1

        selected_candidates = candidates[:limit] if limit > 0 else candidates
        effective_dry_run = dry_run or self.ctx.config.rename.dry_run_default
        print(
            f"--- Rename apply start: {len(selected_candidates)} candidate(s), dry_run={effective_dry_run}"
        )

        ret = self._ensure_tidy_build_ready(app_name)
        if ret != 0:
            print("--- Failed to prepare tidy build for clangd.")
            return ret

        clangd_client = ClangdClient(
            clangd_path=self.ctx.config.rename.clangd_path,
            compile_commands_dir=paths["build_tidy_dir"],
            root_dir=paths["app_dir"],
            background_index=self.ctx.config.rename.clangd_background_index,
        )

        results = []
        status_counts = {"applied": 0, "skipped": 0, "failed": 0}

        try:
            try:
                clangd_client.start()
            except Exception as exc:
                print(f"--- Failed to start clangd: {exc}")
                return 1

            warmup_seconds = max(0.0, float(self.ctx.config.rename.clangd_warmup_seconds))
            if warmup_seconds > 0:
                print(f"--- Waiting {warmup_seconds:.1f}s for clangd index warm-up...")
                time.sleep(warmup_seconds)

            for index, candidate in enumerate(selected_candidates, 1):
                file_path = self._resolve_file_path(
                    candidate.get("file", ""), paths["app_dir"]
                )
                old_name = candidate.get("old_name", "")
                new_name = candidate.get("new_name", "")
                symbol_kind = candidate.get("symbol_kind", "")
                line = int(candidate.get("line", 0))
                col = int(candidate.get("col", 0))

                if not self._is_within_app_scope(file_path, paths["app_dir"]):
                    results.append(
                        {
                            "id": index,
                            "status": "failed",
                            "reason": "out_of_scope_path",
                            "file": str(file_path),
                            "line": line,
                            "col": col,
                            "old_name": old_name,
                            "new_name": new_name,
                            "edit_count": 0,
                            "changed_files": [],
                        }
                    )
                    status_counts["failed"] += 1
                    continue

                if line <= 0 or col <= 0:
                    results.append(
                        {
                            "id": index,
                            "status": "failed",
                            "reason": "invalid_source_location",
                            "file": str(file_path),
                            "line": line,
                            "col": col,
                            "old_name": old_name,
                            "new_name": new_name,
                            "edit_count": 0,
                            "changed_files": [],
                        }
                    )
                    status_counts["failed"] += 1
                    continue

                if not file_path.exists():
                    results.append(
                        {
                            "id": index,
                            "status": "failed",
                            "reason": "file_not_found",
                            "file": str(file_path),
                            "line": line,
                            "col": col,
                            "old_name": old_name,
                            "new_name": new_name,
                            "edit_count": 0,
                            "changed_files": [],
                        }
                    )
                    status_counts["failed"] += 1
                    continue

                file_text = file_path.read_text(encoding="utf-8", errors="replace")
                has_old = bool(
                    old_name and re.search(rf"\b{re.escape(old_name)}\b", file_text)
                )
                has_new = bool(
                    new_name and re.search(rf"\b{re.escape(new_name)}\b", file_text)
                )

                if old_name and not has_old and has_new:
                    results.append(
                        {
                            "id": index,
                            "status": "skipped",
                            "reason": "already_renamed",
                            "file": str(file_path),
                            "line": line,
                            "col": col,
                            "old_name": old_name,
                            "new_name": new_name,
                            "edit_count": 0,
                            "changed_files": [],
                        }
                    )
                    status_counts["skipped"] += 1
                    continue

                resolved_line, resolved_col = self._resolve_position(
                    file_path, line, col, old_name
                )
                preview_result = clangd_client.rename_symbol(
                    file_path=file_path,
                    line=resolved_line,
                    col=resolved_col,
                    new_name=new_name,
                    dry_run=True,
                    allowed_roots=[paths["app_dir"]],
                )

                if not preview_result.get("ok", False):
                    results.append(
                        {
                            "id": index,
                            "status": "failed",
                            "reason": preview_result.get("error", "rename_failed"),
                            "file": str(file_path),
                            "line": resolved_line,
                            "col": resolved_col,
                            "old_name": old_name,
                            "new_name": new_name,
                            "edit_count": 0,
                            "changed_files": [],
                        }
                    )
                    status_counts["failed"] += 1
                    continue

                blocked_files = preview_result.get("blocked_files", [])
                if blocked_files:
                    results.append(
                        {
                            "id": index,
                            "status": "failed",
                            "reason": "out_of_scope_workspace_edit_blocked",
                            "file": str(file_path),
                            "line": resolved_line,
                            "col": resolved_col,
                            "old_name": old_name,
                            "new_name": new_name,
                            "edit_count": 0,
                            "changed_files": [],
                        }
                    )
                    status_counts["failed"] += 1
                    continue

                preview_edit_count = int(preview_result.get("edit_count", 0))
                preview_changed_files = preview_result.get("changed_files", [])
                if self._should_skip_partial_header_rename(
                    symbol_kind=symbol_kind,
                    source_file=file_path,
                    edit_count=preview_edit_count,
                    changed_files=preview_changed_files,
                ):
                    results.append(
                        {
                            "id": index,
                            "status": "skipped",
                            "reason": "skip_risky_header_single_edit",
                            "file": str(file_path),
                            "line": resolved_line,
                            "col": resolved_col,
                            "old_name": old_name,
                            "new_name": new_name,
                            "edit_count": preview_edit_count,
                            "changed_files": preview_changed_files,
                        }
                    )
                    status_counts["skipped"] += 1
                    continue

                if effective_dry_run:
                    rename_result = preview_result
                else:
                    rename_result = clangd_client.rename_symbol(
                        file_path=file_path,
                        line=resolved_line,
                        col=resolved_col,
                        new_name=new_name,
                        dry_run=False,
                        allowed_roots=[paths["app_dir"]],
                    )

                    if not rename_result.get("ok", False):
                        results.append(
                            {
                                "id": index,
                                "status": "failed",
                                "reason": rename_result.get("error", "rename_failed"),
                                "file": str(file_path),
                                "line": resolved_line,
                                "col": resolved_col,
                                "old_name": old_name,
                                "new_name": new_name,
                                "edit_count": 0,
                                "changed_files": [],
                            }
                        )
                        status_counts["failed"] += 1
                        continue

                    blocked_files = rename_result.get("blocked_files", [])
                    if blocked_files:
                        results.append(
                            {
                                "id": index,
                                "status": "failed",
                                "reason": "out_of_scope_workspace_edit_blocked",
                                "file": str(file_path),
                                "line": resolved_line,
                                "col": resolved_col,
                                "old_name": old_name,
                                "new_name": new_name,
                                "edit_count": 0,
                                "changed_files": [],
                            }
                        )
                        status_counts["failed"] += 1
                        continue

                edit_count = int(rename_result.get("edit_count", 0))
                status = "applied" if edit_count > 0 else "skipped"
                reason = "ok" if edit_count > 0 else "no_edit_generated"
                status_counts[status] += 1
                results.append(
                    {
                        "id": index,
                        "status": status,
                        "reason": reason,
                        "file": str(file_path),
                        "line": resolved_line,
                        "col": resolved_col,
                        "old_name": old_name,
                        "new_name": new_name,
                        "edit_count": edit_count,
                        "changed_files": rename_result.get("changed_files", []),
                    }
                )
        finally:
            clangd_client.stop()

        self._write_apply_report(
            json_path=paths["apply_report_json_path"],
            markdown_path=paths["apply_report_md_path"],
            app_name=app_name,
            dry_run=effective_dry_run,
            status_counts=status_counts,
            results=results,
        )

        print("--- Rename apply complete.")
        print(
            f"--- Applied: {status_counts['applied']}, "
            f"Skipped: {status_counts['skipped']}, Failed: {status_counts['failed']}"
        )
        print(f"--- Report JSON: {paths['apply_report_json_path']}")
        print(f"--- Report Markdown: {paths['apply_report_md_path']}")

        if strict and status_counts["failed"] > 0:
            return 1
        return 0

    def audit(self, app_name: str, strict: bool = False) -> int:
        paths = self._paths(app_name)
        candidates = self._load_candidates(paths["candidates_path"])
        if not candidates:
            print("--- No rename candidates found for audit.")
            print("--- Run `python scripts/run.py rename-plan --app <app>` first.")
            return 1

        results = []
        status_counts = {"resolved": 0, "pending": 0, "missing_file": 0, "out_of_scope": 0}

        for index, candidate in enumerate(candidates, 1):
            file_path = self._resolve_file_path(candidate.get("file", ""), paths["app_dir"])
            old_name = candidate.get("old_name", "")
            new_name = candidate.get("new_name", "")
            if not self._is_within_app_scope(file_path, paths["app_dir"]):
                status = "out_of_scope"
                old_count = 0
                new_count = 0
            elif not file_path.exists():
                status = "missing_file"
                old_count = 0
                new_count = 0
            else:
                text = file_path.read_text(encoding="utf-8", errors="replace")
                old_count = self._count_symbol_occurrences(text, old_name)
                new_count = self._count_symbol_occurrences(text, new_name)
                status = "resolved" if old_count == 0 and new_count > 0 else "pending"

                symbol_kind = candidate.get("symbol_kind", "")
                if (
                    status == "resolved"
                    and self._is_header_file(file_path)
                    and self._is_risky_symbol_kind(symbol_kind)
                ):
                    sibling_old_count, sibling_new_count = (
                        self._count_symbol_in_sibling_sources(
                            header_path=file_path,
                            old_name=old_name,
                            new_name=new_name,
                        )
                    )
                    old_count += sibling_old_count
                    new_count += sibling_new_count
                    if sibling_old_count > 0:
                        status = "pending"

            status_counts[status] += 1
            results.append(
                {
                    "id": index,
                    "status": status,
                    "file": str(file_path),
                    "line": int(candidate.get("line", 0)),
                    "col": int(candidate.get("col", 0)),
                    "old_name": old_name,
                    "new_name": new_name,
                    "old_count": old_count,
                    "new_count": new_count,
                }
            )

        self._write_audit_report(
            json_path=paths["audit_report_json_path"],
            markdown_path=paths["audit_report_md_path"],
            app_name=app_name,
            status_counts=status_counts,
            results=results,
        )

        print("--- Rename audit complete.")
        print(
            f"--- Resolved: {status_counts['resolved']}, "
            f"Pending: {status_counts['pending']}, "
            f"Missing file: {status_counts['missing_file']}, "
            f"Out of scope: {status_counts['out_of_scope']}"
        )
        print(f"--- Report JSON: {paths['audit_report_json_path']}")
        print(f"--- Report Markdown: {paths['audit_report_md_path']}")

        if strict and (
            status_counts["pending"] > 0
            or status_counts["missing_file"] > 0
            or status_counts["out_of_scope"] > 0
        ):
            return 1
        return 0

    def _write_apply_report(
        self,
        json_path: Path,
        markdown_path: Path,
        app_name: str,
        dry_run: bool,
        status_counts: Dict[str, int],
        results: List[Dict],
    ):
        json_path.parent.mkdir(parents=True, exist_ok=True)
        payload = {
            "app": app_name,
            "dry_run": dry_run,
            "summary": status_counts,
            "results": results,
        }
        json_path.write_text(
            json.dumps(payload, ensure_ascii=False, indent=2), encoding="utf-8"
        )

        lines = [
            "# Rename Apply Report",
            "",
            f"- App: `{app_name}`",
            f"- Dry run: `{str(dry_run).lower()}`",
            f"- Applied: `{status_counts['applied']}`",
            f"- Skipped: `{status_counts['skipped']}`",
            f"- Failed: `{status_counts['failed']}`",
            "",
            "| ID | Status | File | Line:Col | Old -> New | Edits | Reason |",
            "| --- | --- | --- | --- | --- | --- | --- |",
        ]

        for item in results:
            lines.append(
                "| {id:03d} | {status} | `{file}` | {line}:{col} | `{old}` -> `{new}` | {edits} | {reason} |".format(
                    id=item["id"],
                    status=item["status"],
                    file=item["file"],
                    line=item["line"],
                    col=item["col"],
                    old=item["old_name"],
                    new=item["new_name"],
                    edits=item["edit_count"],
                    reason=item["reason"],
                )
            )

        markdown_path.write_text("\n".join(lines), encoding="utf-8")

    def _write_audit_report(
        self,
        json_path: Path,
        markdown_path: Path,
        app_name: str,
        status_counts: Dict[str, int],
        results: List[Dict],
    ):
        json_path.parent.mkdir(parents=True, exist_ok=True)
        payload = {
            "app": app_name,
            "summary": status_counts,
            "results": results,
        }
        json_path.write_text(
            json.dumps(payload, ensure_ascii=False, indent=2), encoding="utf-8"
        )

        lines = [
            "# Rename Audit Report",
            "",
            f"- App: `{app_name}`",
            f"- Resolved: `{status_counts['resolved']}`",
            f"- Pending: `{status_counts['pending']}`",
            f"- Missing file: `{status_counts['missing_file']}`",
            f"- Out of scope: `{status_counts.get('out_of_scope', 0)}`",
            "",
            "| ID | Status | File | Line:Col | Old -> New | old_count | new_count |",
            "| --- | --- | --- | --- | --- | --- | --- |",
        ]
        for item in results:
            lines.append(
                "| {id:03d} | {status} | `{file}` | {line}:{col} | `{old}` -> `{new}` | {old_count} | {new_count} |".format(
                    id=item["id"],
                    status=item["status"],
                    file=item["file"],
                    line=item["line"],
                    col=item["col"],
                    old=item["old_name"],
                    new=item["new_name"],
                    old_count=item["old_count"],
                    new_count=item["new_count"],
                )
            )
        markdown_path.write_text("\n".join(lines), encoding="utf-8")
