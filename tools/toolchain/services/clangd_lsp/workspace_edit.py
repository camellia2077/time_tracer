from __future__ import annotations

from pathlib import Path

from .codec import position_to_offset


def extract_text_edits(workspace_edit: dict) -> dict[str, list[dict]]:
    edits_by_uri: dict[str, list[dict]] = {}

    for uri, edits in workspace_edit.get("changes", {}).items():
        edits_by_uri.setdefault(uri, []).extend(edits)

    for change in workspace_edit.get("documentChanges", []):
        if "textDocument" in change and "edits" in change:
            uri = change["textDocument"].get("uri", "")
            if uri:
                edits_by_uri.setdefault(uri, []).extend(change["edits"])

    return edits_by_uri


def count_workspace_edit_edits(workspace_edit: dict) -> int:
    edits_by_uri = extract_text_edits(workspace_edit)
    return sum(len(edits) for edits in edits_by_uri.values())


def is_path_in_roots(path: Path, roots: list[Path]) -> bool:
    path_resolved = path.resolve()
    for root in roots:
        try:
            path_resolved.relative_to(root.resolve())
            return True
        except Exception:
            continue
    return False


def apply_text_edits_to_content(text: str, edits: list[dict]) -> str:
    normalized_edits: list[tuple[int, int, str]] = []
    for edit in edits:
        if "range" not in edit:
            continue
        start = edit["range"]["start"]
        end = edit["range"]["end"]
        start_offset = position_to_offset(
            text,
            int(start.get("line", 0)),
            int(start.get("character", 0)),
        )
        end_offset = position_to_offset(
            text,
            int(end.get("line", 0)),
            int(end.get("character", 0)),
        )
        normalized_edits.append((start_offset, end_offset, edit.get("newText", "")))

    normalized_edits.sort(key=lambda item: (item[0], item[1]), reverse=True)
    updated = text
    for start_offset, end_offset, new_text in normalized_edits:
        updated = updated[:start_offset] + new_text + updated[end_offset:]
    return updated
