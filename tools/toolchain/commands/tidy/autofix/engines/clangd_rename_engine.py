from __future__ import annotations

from pathlib import Path

from .....services.clangd_lsp import ClangdClient
from ....cmd_rename.internal.common_symbols import resolve_position
from ...analysis_compile_db import ensure_analysis_compile_db
from ..analyzers import build_diff, has_identifier, line_text
from ..models import ExecutionRecord, FixContext, FixIntent, RenameSymbolOp
from ..reasons import CommonReasons, RenameReasons
from ..rules.identifier_naming import supported_rename_candidate


class ClangdRenameEngine:
    engine_id = "clangd"

    def execute(self, context: FixContext, intents: list[FixIntent]) -> list[ExecutionRecord]:
        compile_db_dir = ensure_analysis_compile_db(context.build_tidy_dir)
        allowed = allowed_roots(context.ctx, context.app_name, context.workspace)
        client = ClangdClient(
            clangd_path=context.ctx.config.rename.clangd_path,
            compile_commands_dir=compile_db_dir,
            root_dir=context.ctx.repo_root,
            background_index=context.ctx.config.rename.clangd_background_index,
        )
        client.start()
        try:
            return [self._execute_intent(context, client, allowed, intent) for intent in intents]
        finally:
            client.stop()

    def _execute_intent(
        self,
        context: FixContext,
        client: ClangdClient,
        allowed: list[Path],
        intent: FixIntent,
    ) -> ExecutionRecord:
        if not isinstance(intent.operation, RenameSymbolOp):
            return ExecutionRecord(
                intent_id=intent.intent_id,
                status="failed",
                reason=CommonReasons.UNSUPPORTED_RENAME_OPERATION,
            )
        file_path = Path(intent.file_path)
        operation = intent.operation
        candidate = {
            "file": str(file_path),
            "line": intent.line,
            "col": intent.col,
            "check": intent.check,
            "symbol_kind": operation.symbol_kind,
            "old_name": operation.old_name,
            "new_name": operation.new_name,
        }
        supported, reason = supported_rename_candidate(
            candidate,
            file_path,
            line_text=line_text(file_path, intent.line),
        )
        if not supported:
            return ExecutionRecord(
                intent_id=intent.intent_id,
                status="skipped",
                reason=reason,
                old_name=operation.old_name,
                new_name=operation.new_name,
            )
        if not file_path.exists():
            return ExecutionRecord(
                intent_id=intent.intent_id,
                status="failed",
                reason=CommonReasons.FILE_NOT_FOUND,
                old_name=operation.old_name,
                new_name=operation.new_name,
            )

        old_name = operation.old_name
        new_name = operation.new_name
        before_text = file_path.read_text(encoding="utf-8", errors="replace")
        source_line = line_text(file_path, intent.line)
        has_old = has_identifier(source_line, old_name)
        has_new = has_identifier(source_line, new_name)
        if old_name and not has_old and has_new:
            return ExecutionRecord(
                intent_id=intent.intent_id,
                status="skipped",
                reason=RenameReasons.ALREADY_RENAMED,
                old_name=old_name,
                new_name=new_name,
            )

        resolved_line, resolved_col = resolve_position(
            file_path,
            intent.line,
            intent.col,
            old_name,
        )
        preview_result = client.rename_symbol(
            file_path=file_path,
            line=resolved_line,
            col=resolved_col,
            new_name=new_name,
            dry_run=True,
            allowed_roots=allowed,
            include_previews=True,
        )
        if not preview_result.get("ok", False):
            return ExecutionRecord(
                intent_id=intent.intent_id,
                status="failed",
                reason=str(preview_result.get("error", RenameReasons.RENAME_FAILED)),
                old_name=old_name,
                new_name=new_name,
            )

        blocked_files = list(preview_result.get("blocked_files", []))
        if blocked_files:
            return ExecutionRecord(
                intent_id=intent.intent_id,
                status="failed",
                reason=RenameReasons.OUT_OF_SCOPE_WORKSPACE_EDIT_BLOCKED,
                old_name=old_name,
                new_name=new_name,
            )

        preview_changed_files = list(preview_result.get("changed_files", []))
        if not within_same_file(preview_changed_files, file_path):
            return ExecutionRecord(
                intent_id=intent.intent_id,
                status="skipped",
                reason=RenameReasons.RENAME_CROSSES_FILE_BOUNDARY,
                changed_files=tuple(preview_changed_files),
                old_name=old_name,
                new_name=new_name,
            )

        edit_count = int(preview_result.get("edit_count", 0))
        preview_entries = list(preview_result.get("previews", []))
        diff = ""
        if preview_entries:
            preview = preview_entries[0]
            diff = build_diff(
                file_path,
                str(preview.get("original_text", "")),
                str(preview.get("updated_text", "")),
            )

        if context.dry_run:
            return ExecutionRecord(
                intent_id=intent.intent_id,
                status="previewed" if edit_count > 0 else "skipped",
                reason=operation.success_reason if edit_count > 0 else CommonReasons.NO_EDIT_GENERATED,
                diff=diff,
                edit_count=edit_count,
                changed_files=tuple(preview_changed_files),
                old_name=old_name,
                new_name=new_name,
            )

        apply_result = client.rename_symbol(
            file_path=file_path,
            line=resolved_line,
            col=resolved_col,
            new_name=new_name,
            dry_run=False,
            allowed_roots=allowed,
        )
        if not apply_result.get("ok", False):
            return ExecutionRecord(
                intent_id=intent.intent_id,
                status="failed",
                reason=str(apply_result.get("error", RenameReasons.RENAME_FAILED)),
                old_name=old_name,
                new_name=new_name,
            )

        changed_files = list(apply_result.get("changed_files", []))
        if not within_same_file(changed_files, file_path):
            return ExecutionRecord(
                intent_id=intent.intent_id,
                status="failed",
                reason=RenameReasons.RENAME_CROSSES_FILE_BOUNDARY,
                changed_files=tuple(changed_files),
                old_name=old_name,
                new_name=new_name,
            )

        after_text = file_path.read_text(encoding="utf-8", errors="replace")
        applied_count = int(apply_result.get("edit_count", 0))
        return ExecutionRecord(
            intent_id=intent.intent_id,
            status="applied" if applied_count > 0 else "skipped",
            reason=operation.success_reason if applied_count > 0 else CommonReasons.NO_EDIT_GENERATED,
            diff=build_diff(file_path, before_text, after_text),
            edit_count=applied_count,
            changed_files=tuple(changed_files),
            old_name=old_name,
            new_name=new_name,
        )


def allowed_roots(ctx, app_name: str, workspace) -> list[Path]:
    if workspace.source_roots:
        return workspace.source_roots
    app_dir = ctx.get_app_dir(app_name)
    if app_dir.exists():
        return [app_dir]
    return [ctx.repo_root]


def within_same_file(changed_files: list[str], file_path: Path) -> bool:
    if not changed_files:
        return False
    resolved_source = file_path.resolve()
    for changed in changed_files:
        if Path(changed).resolve() != resolved_source:
            return False
    return True
