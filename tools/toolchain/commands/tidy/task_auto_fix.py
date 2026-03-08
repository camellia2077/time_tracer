from __future__ import annotations

import difflib
import json
import re
from collections import defaultdict
from dataclasses import asdict, dataclass, field
from pathlib import Path

from ...core.context import Context
from ...services import log_parser
from ...services.clangd_lsp import ClangdClient
from ..cmd_rename.internal.common_symbols import resolve_position
from ..shared import tidy as tidy_shared
from . import analysis_compile_db
from .task_log import ParsedTaskLog, parse_task_log, resolve_task_log_path
from .workspace import ResolvedTidyWorkspace, resolve_workspace

_CPP_SUFFIXES = {".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".hxx"}
_SUPPORTED_RENAME_KINDS = {"constant", "variable"}
_EXACT_NAME_RULES = {
    "payload": "kPayload",
    "parsed": "kParsed",
    "ok": "kOk",
    "found": "kFound",
    "format": "kFormat",
    "root": "kRoot",
    "period": "kPeriod",
    "period_argument": "kPeriodArgument",
    "error_message": "kErrorMessage",
    "error_code": "kErrorCode",
    "error_category": "kErrorCategory",
}
_REDUNDANT_CAST_PATTERN = re.compile(
    r"static_cast<(?P<target>[^>]+)>\((?P<expr>[^()]+)\)"
)
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


@dataclass(slots=True)
class AutoFixAction:
    action_id: str
    kind: str
    file_path: str
    line: int
    col: int
    check: str
    status: str = "pending"
    reason: str = ""
    old_name: str = ""
    new_name: str = ""
    replacement: str = ""
    diff: str = ""
    edit_count: int = 0
    changed_files: list[str] = field(default_factory=list)


@dataclass(slots=True)
class TaskAutoFixResult:
    app_name: str
    task_id: str
    batch_id: str
    task_log: str
    source_file: str
    mode: str
    workspace: str
    source_scope: str | None
    applied: int = 0
    previewed: int = 0
    skipped: int = 0
    failed: int = 0
    action_count: int = 0
    actions: list[AutoFixAction] = field(default_factory=list)
    json_path: str = ""
    markdown_path: str = ""

    def exit_code(self, strict: bool = False) -> int:
        if strict and self.failed > 0:
            return 1
        if self.applied > 0 or self.previewed > 0:
            return 0
        if self.failed > 0:
            return 1
        return 1


def suggest_const_name(old_name: str) -> str:
    normalized = old_name.strip().strip("_")
    if not normalized:
        return ""
    if normalized in _EXACT_NAME_RULES:
        return _EXACT_NAME_RULES[normalized]

    parts = [part for part in normalized.split("_") if part]
    if not parts:
        return ""
    camel = "".join(part[:1].upper() + part[1:] for part in parts)
    return f"k{camel}"


def _line_text(file_path: Path, line_number: int) -> str:
    try:
        lines = file_path.read_text(encoding="utf-8", errors="replace").splitlines()
    except OSError:
        return ""
    if line_number <= 0 or line_number > len(lines):
        return ""
    return lines[line_number - 1]


def _strip_string_literals(text: str) -> str:
    return re.sub(r'"(?:\\.|[^"])*"', '""', text)


def _has_identifier(text: str, name: str) -> bool:
    if not name:
        return False
    return bool(re.search(rf"\b{re.escape(name)}\b", _strip_string_literals(text)))


def _is_cpp_path(file_path: Path) -> bool:
    return file_path.suffix.lower() in _CPP_SUFFIXES


def _within_same_file(changed_files: list[str], file_path: Path) -> bool:
    if not changed_files:
        return False
    resolved_source = file_path.resolve()
    for changed in changed_files:
        if Path(changed).resolve() != resolved_source:
            return False
    return True


def _allowed_roots(
    ctx: Context,
    app_name: str,
    workspace: ResolvedTidyWorkspace,
) -> list[Path]:
    if workspace.source_roots:
        return workspace.source_roots
    app_dir = ctx.get_app_dir(app_name)
    if app_dir.exists():
        return [app_dir]
    return [ctx.repo_root]


def _analysis_compile_db_dir(build_tidy_dir: Path) -> Path:
    return analysis_compile_db.ensure_analysis_compile_db(build_tidy_dir)


def _automation_dir(build_tidy_dir: Path) -> Path:
    return build_tidy_dir / "automation"


def _report_paths(build_tidy_dir: Path, batch_id: str, task_id: str, suffix: str) -> tuple[Path, Path]:
    automation_dir = _automation_dir(build_tidy_dir)
    stem = f"{batch_id}_task_{task_id}_{suffix}"
    return automation_dir / f"{stem}.json", automation_dir / f"{stem}.md"


def _build_diff(file_path: Path, before_text: str, after_text: str) -> str:
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


def _rename_candidates(parsed: ParsedTaskLog) -> list[dict]:
    return log_parser.extract_rename_candidates(
        parsed.diagnostics,
        check_name="readability-identifier-naming",
    )


def _supported_rename_candidate(candidate: dict, file_path: Path) -> tuple[bool, str]:
    symbol_kind = str(candidate.get("symbol_kind", "")).strip().lower()
    if symbol_kind not in _SUPPORTED_RENAME_KINDS:
        return False, f"unsupported_symbol_kind:{symbol_kind or 'unknown'}"
    if not _is_cpp_path(file_path):
        return False, "unsupported_file_type"
    old_name = str(candidate.get("old_name", "")).strip()
    new_name = str(candidate.get("new_name", "")).strip()
    expected_name = suggest_const_name(old_name)
    if not old_name or not new_name:
        return False, "missing_rename_payload"
    if expected_name and new_name != expected_name:
        return False, "rename_not_rule_driven"
    source_line = _line_text(file_path, int(candidate.get("line", 0)))
    if "const " not in source_line and "const auto" not in source_line and "const json" not in source_line:
        return False, "not_const_local_pattern"
    return True, "supported_rule_driven_const_rename"


def plan_redundant_cast_actions(parsed: ParsedTaskLog) -> list[AutoFixAction]:
    source_file = parsed.source_file
    if source_file is None or not source_file.exists():
        return []

    actions: list[AutoFixAction] = []
    source_lines = source_file.read_text(encoding="utf-8", errors="replace").splitlines()
    for index, diagnostic in enumerate(parsed.diagnostics, 1):
        if diagnostic.get("check") != "readability-redundant-casting":
            continue
        message = str(diagnostic.get("message", "")).strip()
        if "same type" not in message:
            continue
        line_number = int(diagnostic.get("line", 0))
        col_number = int(diagnostic.get("col", 0))
        if line_number <= 0 or line_number > len(source_lines):
            continue
        source_line = source_lines[line_number - 1]
        matches = list(_REDUNDANT_CAST_PATTERN.finditer(source_line))
        if not matches:
            continue
        selected_match = None
        source_index = max(0, col_number - 1)
        for match in matches:
            if match.start() <= source_index <= match.end():
                selected_match = match
                break
        if selected_match is None:
            selected_match = matches[0]
        replacement = selected_match.group("expr").strip()
        actions.append(
            AutoFixAction(
                action_id=f"cast:{index:03d}",
                kind="redundant_cast",
                file_path=str(source_file),
                line=line_number,
                col=col_number,
                check="readability-redundant-casting",
                replacement=replacement,
            )
        )
    return actions


def _apply_redundant_casts(
    actions: list[AutoFixAction],
    *,
    dry_run: bool,
) -> None:
    actions_by_file: dict[Path, list[AutoFixAction]] = defaultdict(list)
    for action in actions:
        if action.kind != "redundant_cast":
            continue
        file_path = Path(action.file_path)
        actions_by_file[file_path].append(action)

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
            matches = list(_REDUNDANT_CAST_PATTERN.finditer(source_line))
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

        diff = _build_diff(file_path, before_text, after_text)
        for action in file_actions:
            if action.status in {"previewed", "applied"}:
                action.diff = diff


def _rename_action(
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

    supported, reason = _supported_rename_candidate(candidate, file_path)
    if not supported:
        action.status = "skipped"
        action.reason = reason
        return action
    if not file_path.exists():
        action.status = "failed"
        action.reason = "file_not_found"
        return action

    before_text = file_path.read_text(encoding="utf-8", errors="replace")
    source_line = _line_text(file_path, action.line)
    has_old = _has_identifier(source_line, action.old_name)
    has_new = _has_identifier(source_line, action.new_name)
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
    if not _within_same_file(preview_changed_files, file_path):
        action.status = "skipped"
        action.reason = "rename_crosses_file_boundary"
        action.changed_files = preview_changed_files
        return action

    action.edit_count = int(preview_result.get("edit_count", 0))
    preview_entries = list(preview_result.get("previews", []))
    if preview_entries:
        preview = preview_entries[0]
        action.diff = _build_diff(
            file_path,
            str(preview.get("original_text", "")),
            str(preview.get("updated_text", "")),
        )

    if dry_run:
        action.status = "previewed" if action.edit_count > 0 else "skipped"
        action.reason = "supported_rule_driven_const_rename" if action.edit_count > 0 else "no_edit_generated"
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
    if not _within_same_file(changed_files, file_path):
        action.status = "failed"
        action.reason = "rename_crosses_file_boundary"
        action.changed_files = changed_files
        return action

    after_text = file_path.read_text(encoding="utf-8", errors="replace")
    action.diff = _build_diff(file_path, before_text, after_text)
    action.status = "applied" if int(apply_result.get("edit_count", 0)) > 0 else "skipped"
    action.reason = "supported_rule_driven_const_rename" if action.status == "applied" else "no_edit_generated"
    action.edit_count = int(apply_result.get("edit_count", 0))
    action.changed_files = changed_files
    return action


def _write_result_report(result: TaskAutoFixResult) -> None:
    payload = {
        "app": result.app_name,
        "task_id": result.task_id,
        "batch_id": result.batch_id,
        "task_log": result.task_log,
        "source_file": result.source_file,
        "mode": result.mode,
        "workspace": result.workspace,
        "source_scope": result.source_scope,
        "summary": {
            "applied": result.applied,
            "previewed": result.previewed,
            "skipped": result.skipped,
            "failed": result.failed,
            "action_count": result.action_count,
        },
        "actions": [asdict(action) for action in result.actions],
    }
    json_path = Path(result.json_path)
    markdown_path = Path(result.markdown_path)
    tidy_shared.write_json_dict(json_path, payload)

    md_lines = [
        f"# Task Auto Fix Report ({result.mode})",
        "",
        f"- App: `{result.app_name}`",
        f"- Batch: `{result.batch_id}`",
        f"- Task: `{result.task_id}`",
        f"- Workspace: `{result.workspace}`",
        f"- Source file: `{result.source_file}`",
        f"- Actions: `{result.action_count}`",
        f"- Applied: `{result.applied}`",
        f"- Previewed: `{result.previewed}`",
        f"- Skipped: `{result.skipped}`",
        f"- Failed: `{result.failed}`",
        "",
    ]
    for action in result.actions:
        md_lines.extend(
            [
                f"## {action.action_id} · {action.kind}",
                "",
                f"- File: `{action.file_path}`",
                f"- Check: `{action.check}`",
                f"- Line: `{action.line}:{action.col}`",
                f"- Status: `{action.status}`",
                f"- Reason: `{action.reason}`",
            ]
        )
        if action.old_name or action.new_name:
            md_lines.append(f"- Rename: `{action.old_name}` -> `{action.new_name}`")
        if action.replacement:
            md_lines.append(f"- Replacement: `{action.replacement}`")
        if action.changed_files:
            md_lines.append(
                "- Changed files: "
                + ", ".join(f"`{item}`" for item in action.changed_files)
            )
        if action.diff:
            md_lines.extend(["", "```diff", action.diff, "```"])
        md_lines.append("")

    markdown_path.parent.mkdir(parents=True, exist_ok=True)
    markdown_path.write_text("\n".join(md_lines), encoding="utf-8")


def run_task_auto_fix(
    ctx: Context,
    *,
    app_name: str,
    task_log_path: str | None = None,
    batch_id: str | None = None,
    task_id: str | None = None,
    tidy_build_dir_name: str | None = None,
    source_scope: str | None = None,
    dry_run: bool = False,
    report_suffix: str = "fix",
) -> TaskAutoFixResult:
    workspace = resolve_workspace(
        ctx,
        build_dir_name=tidy_build_dir_name,
        source_scope=source_scope,
    )
    app_dir = ctx.get_app_dir(app_name)
    build_tidy_dir = app_dir / workspace.build_dir_name
    tasks_dir = build_tidy_dir / "tasks"
    resolved_task_path = resolve_task_log_path(
        tasks_dir,
        task_log_path=task_log_path,
        batch_id=batch_id,
        task_id=task_id,
    )
    parsed = parse_task_log(resolved_task_path)
    source_file = parsed.source_file or resolved_task_path
    json_path, markdown_path = _report_paths(
        build_tidy_dir,
        parsed.batch_id or "batch_unknown",
        parsed.task_id,
        report_suffix,
    )

    result = TaskAutoFixResult(
        app_name=app_name,
        task_id=parsed.task_id,
        batch_id=parsed.batch_id,
        task_log=str(resolved_task_path),
        source_file=str(source_file),
        mode="dry_run" if dry_run else "apply",
        workspace=workspace.build_dir_name,
        source_scope=workspace.source_scope,
        json_path=str(json_path),
        markdown_path=str(markdown_path),
    )

    rename_candidates = _rename_candidates(parsed)
    cast_actions = plan_redundant_cast_actions(parsed)
    result.action_count = len(rename_candidates) + len(cast_actions)

    if rename_candidates:
        try:
            compile_db_dir = _analysis_compile_db_dir(build_tidy_dir)
            allowed_roots = _allowed_roots(ctx, app_name, workspace)
            client = ClangdClient(
                clangd_path=ctx.config.rename.clangd_path,
                compile_commands_dir=compile_db_dir,
                root_dir=ctx.repo_root,
                background_index=ctx.config.rename.clangd_background_index,
            )
            client.start()
            try:
                for index, candidate in enumerate(rename_candidates, 1):
                    action = _rename_action(
                        ctx,
                        client=client,
                        candidate=candidate,
                        allowed_roots=allowed_roots,
                        dry_run=dry_run,
                        action_index=index,
                    )
                    result.actions.append(action)
            finally:
                client.stop()
        except Exception as exc:
            result.actions.append(
                AutoFixAction(
                    action_id="rename:bootstrap",
                    kind="rename",
                    file_path=str(source_file),
                    line=0,
                    col=0,
                    check="readability-identifier-naming",
                    status="failed",
                    reason=f"clangd_bootstrap_failed:{exc}",
                )
            )

    if cast_actions:
        _apply_redundant_casts(cast_actions, dry_run=dry_run)
        result.actions.extend(cast_actions)

    for action in result.actions:
        if action.status == "applied":
            result.applied += 1
        elif action.status == "previewed":
            result.previewed += 1
        elif action.status == "failed":
            result.failed += 1
        else:
            result.skipped += 1

    _write_result_report(result)
    return result


def suggest_task_refactors(parsed: ParsedTaskLog) -> list[dict]:
    source_file = parsed.source_file
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
        has_deref_value = re.search(
            _DEREF_VALUE_PATTERN_TEMPLATE.format(var=re.escape(var_name)),
            content,
        )
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
                "evidence": {
                    "variable": variable_name,
                    "values": unique_values,
                },
            }
        )

    return suggestions


def write_task_suggestion_report(
    *,
    build_tidy_dir: Path,
    app_name: str,
    parsed: ParsedTaskLog,
    workspace_name: str,
    source_scope: str | None,
    suggestions: list[dict],
) -> tuple[Path, Path]:
    json_path, markdown_path = _report_paths(
        build_tidy_dir,
        parsed.batch_id or "batch_unknown",
        parsed.task_id,
        "suggest",
    )
    payload = {
        "app": app_name,
        "task_id": parsed.task_id,
        "batch_id": parsed.batch_id,
        "task_log": str(parsed.task_path),
        "source_file": str(parsed.source_file or ""),
        "workspace": workspace_name,
        "source_scope": source_scope,
        "suggestions": suggestions,
    }
    tidy_shared.write_json_dict(json_path, payload)

    md_lines = [
        "# Task Refactor Suggestions",
        "",
        f"- App: `{app_name}`",
        f"- Batch: `{parsed.batch_id}`",
        f"- Task: `{parsed.task_id}`",
        f"- Workspace: `{workspace_name}`",
        f"- Source file: `{parsed.source_file or ''}`",
        "",
    ]
    if not suggestions:
        md_lines.append("- No structured suggestions generated.")
    else:
        for index, suggestion in enumerate(suggestions, 1):
            md_lines.extend(
                [
                    f"## Suggestion {index}: {suggestion.get('kind', 'unknown')}",
                    "",
                    f"- Priority: `{suggestion.get('priority', 'unknown')}`",
                    f"- Summary: {suggestion.get('summary', '')}",
                    f"- Evidence: `{json.dumps(suggestion.get('evidence', []), ensure_ascii=False)}`",
                    "",
                ]
            )
    markdown_path.parent.mkdir(parents=True, exist_ok=True)
    markdown_path.write_text("\n".join(md_lines), encoding="utf-8")
    return json_path, markdown_path
