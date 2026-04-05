from __future__ import annotations

import json
from dataclasses import asdict
from pathlib import Path

from ...shared import tidy as tidy_shared


def report_paths(build_tidy_dir: Path, batch_id: str, task_id: str, suffix: str) -> tuple[Path, Path]:
    automation_dir = build_tidy_dir / "automation"
    stem = f"{batch_id}_task_{task_id}_{suffix}"
    return automation_dir / f"{stem}.json", automation_dir / f"{stem}.md"


def write_result_report(result) -> None:
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
                f"- Rule: `{action.rule_id or action.kind}`",
                f"- Line: `{action.line}:{action.col}`",
                f"- Risk: `{action.risk_level or 'unknown'}`",
                f"- Mode: `{'preview-only' if action.preview_only else 'apply'}`",
                f"- Status: `{action.status}`",
                f"- Reason: `{action.reason}`",
            ]
        )
        if action.old_name or action.new_name:
            change_label = "Rename" if action.kind == "rename" else "Change"
            md_lines.append(f"- {change_label}: `{action.old_name}` -> `{action.new_name}`")
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


def suggest_task_refactors(parsed, *, detect_task_refactors_fn) -> list[dict]:
    return detect_task_refactors_fn(parsed)


def write_task_suggestion_report(
    *,
    build_tidy_dir: Path,
    app_name: str,
    parsed,
    task_path: Path,
    workspace_name: str,
    source_scope: str | None,
    suggestions: list[dict],
) -> tuple[Path, Path]:
    json_path, markdown_path = report_paths(
        build_tidy_dir,
        parsed.batch_id or "batch_unknown",
        parsed.task_id,
        "suggest",
    )
    payload = {
        "app": app_name,
        "task_id": parsed.task_id,
        "batch_id": parsed.batch_id,
        "task_log": str(task_path),
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
