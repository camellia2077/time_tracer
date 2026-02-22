from __future__ import annotations


def normalize_rel_path(raw_path: str) -> str:
    path = raw_path.strip().replace("\\", "/")
    while path.startswith("./"):
        path = path[2:]
    if path.startswith("config/"):
        path = path[len("config/") :]
    return path


def dedupe_keep_order(items: list[str]) -> list[str]:
    seen: set[str] = set()
    output: list[str] = []
    for item in items:
        if item in seen:
            continue
        seen.add(item)
        output.append(item)
    return output
