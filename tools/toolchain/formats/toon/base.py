from __future__ import annotations


def escape_cell(text: object, *, trim: bool = True) -> str:
    escaped = str(text or "").replace("\n", " ").replace(",", ";")
    return escaped.strip() if trim else escaped


def split_row(
    value: str,
    *,
    expected_parts: int,
    maxsplit: int | None = None,
) -> list[str] | None:
    actual_maxsplit = expected_parts - 1 if maxsplit is None else maxsplit
    parts = value.split(",", actual_maxsplit)
    if len(parts) != expected_parts:
        return None
    return [part.strip() for part in parts]
