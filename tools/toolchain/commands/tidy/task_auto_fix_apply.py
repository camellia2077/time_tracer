from __future__ import annotations

import difflib
import re
from collections import defaultdict
from pathlib import Path

from ...core.context import Context
from ...services.clangd_lsp import ClangdClient
from ..cmd_rename.internal.common_symbols import resolve_position
from . import analysis_compile_db
from .task_auto_fix_plan import rename_candidates, suggest_const_name, supported_rename_candidate
from .task_auto_fix_types import AutoFixAction

_READ_FIELD_PATTERN = re.compile(
    r"^\s*const\s+(?:auto|json|bool|int|double|std::[A-Za-z_:<>]+)\s+"
    r"(?P<name>[A-Za-z_][A-Za-z0-9_]*)\s*=\s*TryRead[A-Za-z0-9_]+Field\(",
    flags=re.MULTILINE,
)
_HAS_ERROR_PATTERN_TEMPLATE = r"\b{var}\.HasError\(\)"
_HAS_VALUE_PATTERN_TEMPLATE = r"\b{var}\.value\.has_value\(\)"
_VALUE_OR_PATTERN_TEMPLATE = r"\b{var}\.value\.value_or\("
_DEREF_VALUE_PATTERN_TEMPLATE = r"\*\s*{var}\.value\b"
_MAGIC_IF_PATTERN = re.compile(
    r"if\s*\(\s*(?P<var>[A-Za-z_][A-Za-z0-9_]*)\s*==\s*(?P<num>\d+)\s*\)"
)


def line_text(file_path: Path, line_number: int) -> str:
    try:
        lines = file_path.read_text(encoding="utf-8", errors="replace").splitlines()
    except OSError:
        return ""
    if line_number <= 0 or line_number > len(lines):
        return ""
    return lines[line_number - 1]


def strip_string_literals(text: str) -> str:
    return re.sub(r'"(?:\\.|[^"])*"', '""', text)


def has_identifier(text: str, name: str) -> bool:
    if not name:
        return False
    return bool(re.search(rf"\b{re.escape(name)}\b", strip_string_literals(text)))


def within_same_file(changed_files: list[str], file_path: Path) -> bool:
    if not changed_files:
        return False
    resolved_source = file_path.resolve()
    for changed in changed_files:
        if Path(changed).resolve() != resolved_source:
            return False
    return True


def allowed_roots(
    ctx: Context,
    app_name: str,
    workspace,
) -> list[Path]:
    if workspace.source_roots:
        return workspace.source_roots
    app_dir = ctx.get_app_dir(app_name)
    if app_dir.exists():
        return [app_dir]
    return [ctx.repo_root]


def analysis_compile_db_dir(build_tidy_dir: Path) -> Path:
    return analysis_compile_db.ensure_analysis_compile_db(build_tidy_dir)


def build_diff(file_path: Path, before_text: str, after_text: str) -> str:
    if before_text == after_text:
        return ""
    diff_lines = difflib.unified_diff(
        before_text.splitlines(),
        after_text.splitlines(),
        fromfile=str(file_path),
        tofile=str(file_path),
        lineterm="",
    )
    return "\n".join(diff_lines)


def apply_redundant_casts(
    actions: list[AutoFixAction],
    *,
    dry_run: bool,
) -> None:
    pattern = re.compile(r"static_cast<(?P<target>[^>]+)>\((?P<expr>[^()]+)\)")
    actions_by_file: dict[Path, list[AutoFixAction]] = defaultdict(list)
    for action in actions:
        if action.kind != "redundant_cast":
            continue
        actions_by_file[Path(action.file_path)].append(action)

    for file_path, file_actions in actions_by_file.items():
        if not file_path.exists():
            for action in file_actions:
                action.status = "failed"
                action.reason = "file_not_found"
            continue

        before_text = file_path.read_text(encoding="utf-8", errors="replace")
        lines = before_text.splitlines()
        replacements: list[tuple[int, int, int, str, AutoFixAction]] = []
        for action in file_actions:
            line_index = action.line - 1
            if line_index < 0 or line_index >= len(lines):
                action.status = "failed"
                action.reason = "invalid_line"
                continue
            source_line = lines[line_index]
            matches = list(pattern.finditer(source_line))
            if not matches:
                action.status = "skipped"
                action.reason = "no_safe_same_line_cast_match"
                continue
            source_index = max(0, action.col - 1)
            selected_match = None
            for match in matches:
                if match.start() <= source_index <= match.end():
                    selected_match = match
                    break
            if selected_match is None:
                selected_match = matches[0]
            replacements.append(
                (
                    line_index,
                    selected_match.start(),
                    selected_match.end(),
                    selected_match.group("expr").strip(),
                    action,
                )
            )

        replacements.sort(key=lambda item: (item[0], item[1]), reverse=True)
        updated_lines = list(lines)
        for line_index, start_col, end_col, replacement, action in replacements:
            current_line = updated_lines[line_index]
            updated_lines[line_index] = current_line[:start_col] + replacement + current_line[end_col:]
            action.status = "previewed" if dry_run else "applied"
            action.reason = "safe_same_type_cast_removed"
            action.edit_count = 1
            action.changed_files = [str(file_path)]

        after_text = "\n".join(updated_lines)
        if before_text.endswith("\n"):
            after_text += "\n"
        if not dry_run and before_text != after_text:
            file_path.write_text(after_text, encoding="utf-8")

        diff = build_diff(file_path, before_text, after_text)
        for action in file_actions:
            if action.status in {"previewed", "applied"}:
                action.diff = diff


def rename_action(
    ctx: Context,
    *,
    client: ClangdClient,
    candidate: dict,
    allowed_roots: list[Path],
    dry_run: bool,
    action_index: int,
) -> AutoFixAction:
    file_path = Path(str(candidate.get("file", "")))
    action = AutoFixAction(
        action_id=f"rename:{action_index:03d}",
        kind="rename",
        file_path=str(file_path),
        line=int(candidate.get("line", 0)),
        col=int(candidate.get("col", 0)),
        check=str(candidate.get("check", "")),
        old_name=str(candidate.get("old_name", "")),
        new_name=str(candidate.get("new_name", "")),
    )

    supported, reason = supported_rename_candidate(
        candidate,
        file_path,
        line_text=line_text(file_path, action.line),
    )
    if not supported:
        action.status = "skipped"
        action.reason = reason
        return action
    if not file_path.exists():
        action.status = "failed"
        action.reason = "file_not_found"
        return action

    before_text = file_path.read_text(encoding="utf-8", errors="replace")
    source_line = line_text(file_path, action.line)
    has_old = has_identifier(source_line, action.old_name)
    has_new = has_identifier(source_line, action.new_name)
    if action.old_name and not has_old and has_new:
        action.status = "skipped"
        action.reason = "already_renamed"
        return action

    resolved_line, resolved_col = resolve_position(
        file_path,
        action.line,
        action.col,
        action.old_name,
    )
    preview_result = client.rename_symbol(
        file_path=file_path,
        line=resolved_line,
        col=resolved_col,
        new_name=action.new_name,
        dry_run=True,
        allowed_roots=allowed_roots,
        include_previews=True,
    )
    if not preview_result.get("ok", False):
        action.status = "failed"
        action.reason = str(preview_result.get("error", "rename_failed"))
        return action

    blocked_files = list(preview_result.get("blocked_files", []))
    if blocked_files:
        action.status = "failed"
        action.reason = "out_of_scope_workspace_edit_blocked"
        return action

    preview_changed_files = list(preview_result.get("changed_files", []))
    if not within_same_file(preview_changed_files, file_path):
        action.status = "skipped"
        action.reason = "rename_crosses_file_boundary"
        action.changed_files = preview_changed_files
        return action

    action.edit_count = int(preview_result.get("edit_count", 0))
    preview_entries = list(preview_result.get("previews", []))
    if preview_entries:
        preview = preview_entries[0]
        action.diff = build_diff(
            file_path,
            str(preview.get("original_text", "")),
            str(preview.get("updated_text", "")),
        )

    if dry_run:
        action.status = "previewed" if action.edit_count > 0 else "skipped"
        action.reason = (
            "supported_rule_driven_const_rename"
            if action.edit_count > 0
            else "no_edit_generated"
        )
        action.changed_files = preview_changed_files
        return action

    apply_result = client.rename_symbol(
        file_path=file_path,
        line=resolved_line,
        col=resolved_col,
        new_name=action.new_name,
        dry_run=False,
        allowed_roots=allowed_roots,
    )
    if not apply_result.get("ok", False):
        action.status = "failed"
        action.reason = str(apply_result.get("error", "rename_failed"))
        return action

    changed_files = list(apply_result.get("changed_files", []))
    if not within_same_file(changed_files, file_path):
        action.status = "failed"
        action.reason = "rename_crosses_file_boundary"
        action.changed_files = changed_files
        return action

    after_text = file_path.read_text(encoding="utf-8", errors="replace")
    action.diff = build_diff(file_path, before_text, after_text)
    action.status = "applied" if int(apply_result.get("edit_count", 0)) > 0 else "skipped"
    action.reason = (
        "supported_rule_driven_const_rename"
        if action.status == "applied"
        else "no_edit_generated"
    )
    action.edit_count = int(apply_result.get("edit_count", 0))
    action.changed_files = changed_files
    return action


def detect_task_refactors(parsed) -> list[dict]:
    source_file = Path(parsed.source_file) if parsed.source_file else None
    if source_file is None or not source_file.exists():
        return []

    content = source_file.read_text(encoding="utf-8", errors="replace")
    suggestions: list[dict] = []

    decode_vars = [match.group("name") for match in _READ_FIELD_PATTERN.finditer(content)]
    decode_ready_vars: list[str] = []
    for var_name in decode_vars:
        has_error = re.search(_HAS_ERROR_PATTERN_TEMPLATE.format(var=re.escape(var_name)), content)
        has_value = re.search(_HAS_VALUE_PATTERN_TEMPLATE.format(var=re.escape(var_name)), content)
        has_value_or = re.search(_VALUE_OR_PATTERN_TEMPLATE.format(var=re.escape(var_name)), content)
        has_deref_value = re.search(_DEREF_VALUE_PATTERN_TEMPLATE.format(var=re.escape(var_name)), content)
        if has_error and (has_value or has_value_or or has_deref_value):
            decode_ready_vars.append(var_name)
    if len(decode_ready_vars) >= 3:
        suggestions.append(
            {
                "kind": "decode_helper",
                "priority": "medium",
                "summary": "重复的 TryRead/HasError/value.has_value 骨架适合抽成 decode helper。",
                "evidence": decode_ready_vars[:8],
            }
        )

    magic_hits: dict[str, list[str]] = defaultdict(list)
    for match in _MAGIC_IF_PATTERN.finditer(content):
        magic_hits[match.group("var")].append(match.group("num"))
    for variable_name, values in sorted(magic_hits.items()):
        unique_values = sorted(set(values), key=int)
        if len(unique_values) < 3:
            continue
        suggestions.append(
            {
                "kind": "protocol_constants",
                "priority": "medium",
                "summary": "链式数值分支看起来是协议码/枚举映射，适合提取命名常量或 enum。",
                "evidence": {"variable": variable_name, "values": unique_values},
            }
        )

    return suggestions
