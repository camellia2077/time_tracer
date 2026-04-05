from __future__ import annotations


class CommonReasons:
    NO_EDIT_GENERATED = "no_edit_generated"
    FILE_NOT_FOUND = "file_not_found"
    FILE_READ_FAILED = "file_read_failed"
    INVALID_LINE = "invalid_line"
    APPLY_NOT_ENABLED_PREVIEW_ONLY_RULE = "apply_not_enabled_preview_only_rule"
    OVERLAPPING_WORKSPACE_EDITS = "overlapping_workspace_edits"
    UNSUPPORTED_TEXT_OPERATION = "unsupported_text_operation"
    UNSUPPORTED_RENAME_OPERATION = "unsupported_rename_operation"
    UNSUPPORTED_ENGINE = "unsupported_engine"
    MISSING_EXECUTION_RECORD = "missing_execution_record"


class RenameReasons:
    ALREADY_RENAMED = "already_renamed"
    RENAME_FAILED = "rename_failed"
    OUT_OF_SCOPE_WORKSPACE_EDIT_BLOCKED = "out_of_scope_workspace_edit_blocked"
    RENAME_CROSSES_FILE_BOUNDARY = "rename_crosses_file_boundary"
    SUPPORTED_RULE_DRIVEN_CONST_RENAME = "supported_rule_driven_const_rename"
