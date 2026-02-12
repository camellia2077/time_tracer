import json
import re
from pathlib import Path
from typing import Dict, List, Optional

from . import log_parser

TASK_FILE_PATTERN = re.compile(r"task_(\d+)\.log$")


def _task_id_from_path(path: Path) -> str:
    match = TASK_FILE_PATTERN.search(path.name)
    if not match:
        return "000"
    return match.group(1).zfill(3)


def _normalize_file_path(path_str: str) -> str:
    if not path_str:
        return path_str
    return str(Path(path_str))


def _is_within_root(path: Path, root: Path) -> bool:
    try:
        path.resolve().relative_to(root.resolve())
        return True
    except Exception:
        return False


def collect_rename_candidates(
    tasks_dir: Path,
    check_name: str,
    allowed_kinds: Optional[List[str]] = None,
    app_root: Optional[Path] = None,
) -> List[Dict]:
    if not tasks_dir.exists():
        return []

    allowed_kind_set = {kind.lower() for kind in (allowed_kinds or [])}
    collected: List[Dict] = []
    dedupe_key_set = set()

    task_files = list(tasks_dir.rglob("task_*.log"))
    task_files.sort(key=lambda path: (_task_id_from_path(path), str(path)))
    for task_file in task_files:
        task_id = _task_id_from_path(task_file)
        content = task_file.read_text(encoding="utf-8", errors="replace")
        diagnostics = log_parser.extract_diagnostics(content.splitlines())
        candidates = log_parser.extract_rename_candidates(
            diagnostics, check_name=check_name
        )

        for candidate in candidates:
            symbol_kind = candidate.get("symbol_kind", "").strip().lower()
            if allowed_kind_set and symbol_kind not in allowed_kind_set:
                continue

            file_path = _normalize_file_path(candidate.get("file", ""))
            file_path_obj = Path(file_path)
            if app_root and not file_path_obj.is_absolute():
                file_path_obj = app_root / file_path_obj

            if app_root and not _is_within_root(file_path_obj, app_root):
                continue

            normalized_path = str(file_path_obj.resolve()) if app_root else str(file_path_obj)
            dedupe_key = (
                normalized_path.lower(),
                int(candidate.get("line", 0)),
                int(candidate.get("col", 0)),
                candidate.get("new_name", ""),
            )
            if dedupe_key in dedupe_key_set:
                continue
            dedupe_key_set.add(dedupe_key)

            collected.append(
                {
                    "task_id": task_id,
                    "file": normalized_path,
                    "line": int(candidate.get("line", 0)),
                    "col": int(candidate.get("col", 0)),
                    "check": candidate.get("check", ""),
                    "symbol_kind": candidate.get("symbol_kind", ""),
                    "old_name": candidate.get("old_name", ""),
                    "new_name": candidate.get("new_name", ""),
                    "message": candidate.get("message", ""),
                    "source_task_log": str(task_file),
                }
            )

    collected.sort(key=lambda item: (item["task_id"], item["file"], item["line"], item["col"]))
    return collected


def write_plan_outputs(
    rename_dir: Path,
    all_candidates: List[Dict],
    check_name: str,
    max_candidates: Optional[int],
) -> Dict:
    rename_dir.mkdir(parents=True, exist_ok=True)

    limit = max_candidates if max_candidates and max_candidates > 0 else len(all_candidates)
    selected_candidates = all_candidates[:limit]
    truncated = len(selected_candidates) < len(all_candidates)

    json_path = rename_dir / "rename_candidates.json"
    json_content = {
        "check_name": check_name,
        "total_candidates": len(all_candidates),
        "selected_candidates": len(selected_candidates),
        "truncated": truncated,
        "candidates": selected_candidates,
    }
    json_path.write_text(
        json.dumps(json_content, ensure_ascii=False, indent=2), encoding="utf-8"
    )

    md_path = rename_dir / "rename_plan.md"
    md_lines = [
        "# Rename Plan",
        "",
        f"- Check: `{check_name}`",
        f"- Total candidates found: `{len(all_candidates)}`",
        f"- Candidates selected: `{len(selected_candidates)}`",
        f"- Truncated by limit: `{str(truncated).lower()}`",
        "",
        "| ID | Task | Kind | File | Line:Col | Old -> New |",
        "| --- | --- | --- | --- | --- | --- |",
    ]

    for index, candidate in enumerate(selected_candidates, 1):
        md_lines.append(
            "| {id:03d} | {task} | {kind} | `{file}` | {line}:{col} | `{old}` -> `{new}` |".format(
                id=index,
                task=candidate.get("task_id", "000"),
                kind=candidate.get("symbol_kind", ""),
                file=candidate.get("file", ""),
                line=candidate.get("line", 0),
                col=candidate.get("col", 0),
                old=candidate.get("old_name", ""),
                new=candidate.get("new_name", ""),
            )
        )

    md_path.write_text("\n".join(md_lines), encoding="utf-8")

    return {
        "selected_candidates": selected_candidates,
        "json_path": json_path,
        "md_path": md_path,
        "total_candidates": len(all_candidates),
        "truncated": truncated,
    }
