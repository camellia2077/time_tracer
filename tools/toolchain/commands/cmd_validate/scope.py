from __future__ import annotations

from pathlib import Path


def _to_repo_relative(repo_root: Path, raw_path: str) -> str:
    candidate = Path(raw_path)
    if not candidate.is_absolute():
        candidate = (repo_root / candidate).resolve()
    else:
        candidate = candidate.resolve()

    try:
        return str(candidate.relative_to(repo_root.resolve())).replace("\\", "/")
    except ValueError:
        return str(candidate)


def resolve_scope_paths(
    repo_root: Path,
    raw_paths: list[str] | None,
    paths_file: str | None,
) -> list[str]:
    collected: list[str] = []

    for item in raw_paths or []:
        text = str(item).strip()
        if text:
            collected.append(text)

    if paths_file:
        file_path = Path(paths_file)
        if not file_path.is_absolute():
            file_path = repo_root / file_path
        if not file_path.exists():
            raise FileNotFoundError(f"paths file not found: {file_path.resolve()}")
        for line in file_path.read_text(encoding="utf-8", errors="replace").splitlines():
            text = line.strip()
            if not text or text.startswith("#"):
                continue
            collected.append(text)

    if not collected:
        raise ValueError("validate requires --paths or --paths-file")

    normalized: list[str] = []
    seen: set[str] = set()
    for raw_path in collected:
        normalized_path = _to_repo_relative(repo_root, raw_path)
        if normalized_path in seen:
            continue
        seen.add(normalized_path)
        normalized.append(normalized_path)
    return normalized
