import re
from pathlib import Path

from ...services.clangd_lsp import ClangdClient


def process_apply_candidate(
    command,
    index: int,
    candidate: dict,
    app_dir: Path,
    clangd_client: ClangdClient,
    effective_dry_run: bool,
) -> dict:
    (
        file_path,
        old_name,
        new_name,
        symbol_kind,
        line,
        col,
    ) = command._extract_candidate_fields(candidate, app_dir)

    if not command._is_within_app_scope(file_path, app_dir):
        return command._build_apply_result(
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
        return command._build_apply_result(
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
        return command._build_apply_result(
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
        return command._build_apply_result(
            index,
            "skipped",
            "already_renamed",
            file_path,
            line,
            col,
            old_name,
            new_name,
        )

    resolved_line, resolved_col = command._resolve_position(file_path, line, col, old_name)
    preview_result = clangd_client.rename_symbol(
        file_path=file_path,
        line=resolved_line,
        col=resolved_col,
        new_name=new_name,
        dry_run=True,
        allowed_roots=[app_dir],
    )
    if not preview_result.get("ok", False):
        return command._build_apply_result(
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
        return command._build_apply_result(
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
    if command._should_skip_partial_header_rename(
        symbol_kind=symbol_kind,
        source_file=file_path,
        edit_count=preview_edit_count,
        changed_files=preview_changed_files,
    ):
        return command._build_apply_result(
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
            return command._build_apply_result(
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
            return command._build_apply_result(
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
    return command._build_apply_result(
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
