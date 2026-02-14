import re
from pathlib import Path
from typing import Dict

from ..services.clangd_lsp import ClangdClient


class RenameApplyMixin:
    def _process_apply_candidate(
        self,
        index: int,
        candidate: Dict,
        app_dir: Path,
        clangd_client: ClangdClient,
        effective_dry_run: bool,
    ) -> Dict:
        (
            file_path,
            old_name,
            new_name,
            symbol_kind,
            line,
            col,
        ) = self._extract_candidate_fields(candidate, app_dir)

        if not self._is_within_app_scope(file_path, app_dir):
            return self._build_apply_result(
                index,
                "failed",
                "out_of_scope_path",
                file_path,
                line,
                col,
                old_name,
                new_name,
            )

        if line <= 0 or col <= 0:
            return self._build_apply_result(
                index,
                "failed",
                "invalid_source_location",
                file_path,
                line,
                col,
                old_name,
                new_name,
            )

        if not file_path.exists():
            return self._build_apply_result(
                index,
                "failed",
                "file_not_found",
                file_path,
                line,
                col,
                old_name,
                new_name,
            )

        file_text = file_path.read_text(encoding="utf-8", errors="replace")
        has_old = bool(old_name and re.search(rf"\b{re.escape(old_name)}\b", file_text))
        has_new = bool(new_name and re.search(rf"\b{re.escape(new_name)}\b", file_text))
        if old_name and not has_old and has_new:
            return self._build_apply_result(
                index,
                "skipped",
                "already_renamed",
                file_path,
                line,
                col,
                old_name,
                new_name,
            )

        resolved_line, resolved_col = self._resolve_position(file_path, line, col, old_name)
        preview_result = clangd_client.rename_symbol(
            file_path=file_path,
            line=resolved_line,
            col=resolved_col,
            new_name=new_name,
            dry_run=True,
            allowed_roots=[app_dir],
        )
        if not preview_result.get("ok", False):
            return self._build_apply_result(
                index,
                "failed",
                preview_result.get("error", "rename_failed"),
                file_path,
                resolved_line,
                resolved_col,
                old_name,
                new_name,
            )

        preview_blocked_files = preview_result.get("blocked_files", [])
        if preview_blocked_files:
            return self._build_apply_result(
                index,
                "failed",
                "out_of_scope_workspace_edit_blocked",
                file_path,
                resolved_line,
                resolved_col,
                old_name,
                new_name,
            )

        preview_edit_count = int(preview_result.get("edit_count", 0))
        preview_changed_files = preview_result.get("changed_files", [])
        if self._should_skip_partial_header_rename(
            symbol_kind=symbol_kind,
            source_file=file_path,
            edit_count=preview_edit_count,
            changed_files=preview_changed_files,
        ):
            return self._build_apply_result(
                index,
                "skipped",
                "skip_risky_header_single_edit",
                file_path,
                resolved_line,
                resolved_col,
                old_name,
                new_name,
                edit_count=preview_edit_count,
                changed_files=preview_changed_files,
            )

        rename_result = preview_result
        if not effective_dry_run:
            rename_result = clangd_client.rename_symbol(
                file_path=file_path,
                line=resolved_line,
                col=resolved_col,
                new_name=new_name,
                dry_run=False,
                allowed_roots=[app_dir],
            )
            if not rename_result.get("ok", False):
                return self._build_apply_result(
                    index,
                    "failed",
                    rename_result.get("error", "rename_failed"),
                    file_path,
                    resolved_line,
                    resolved_col,
                    old_name,
                    new_name,
                )

            blocked_files = rename_result.get("blocked_files", [])
            if blocked_files:
                return self._build_apply_result(
                    index,
                    "failed",
                    "out_of_scope_workspace_edit_blocked",
                    file_path,
                    resolved_line,
                    resolved_col,
                    old_name,
                    new_name,
                )

        edit_count = int(rename_result.get("edit_count", 0))
        status = "applied" if edit_count > 0 else "skipped"
        reason = "ok" if edit_count > 0 else "no_edit_generated"
        return self._build_apply_result(
            index,
            status,
            reason,
            file_path,
            resolved_line,
            resolved_col,
            old_name,
            new_name,
            edit_count=edit_count,
            changed_files=rename_result.get("changed_files", []),
        )

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
            if not self._start_clangd(clangd_client):
                return 1

            self._wait_for_clangd_warmup()

            for index, candidate in enumerate(selected_candidates, 1):
                result = self._process_apply_candidate(
                    index=index,
                    candidate=candidate,
                    app_dir=paths["app_dir"],
                    clangd_client=clangd_client,
                    effective_dry_run=effective_dry_run,
                )
                status_counts[result["status"]] += 1
                results.append(result)
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
