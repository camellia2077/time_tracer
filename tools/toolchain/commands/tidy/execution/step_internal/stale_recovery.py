def can_continue_after_fix_failures(fix_result) -> bool:
    allowed_rename_failure_markers = (
        "Cannot rename symbol: there is no symbol at the given location",
        "Cannot rename symbol: symbol is not a supported kind",
    )
    if fix_result.failed <= 0:
        return True
    failed_actions = [action for action in fix_result.actions if action.status == "failed"]
    if not failed_actions:
        return False
    for action in failed_actions:
        reason = str(action.reason or "")
        if action.kind != "rename" or not any(
            marker in reason for marker in allowed_rename_failure_markers
        ):
            return False
    return True
