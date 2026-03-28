from __future__ import annotations

import os
from pathlib import Path


def _same_location(left: Path, right: Path) -> bool:
    if left.exists() and right.exists():
        try:
            return left.samefile(right)
        except OSError:
            pass

    left_norm = os.path.normcase(str(left.resolve(strict=False)))
    right_norm = os.path.normcase(str(right.resolve(strict=False)))
    return left_norm == right_norm


def to_repo_relative(repo_root: Path, path: Path) -> str:
    resolved_root = repo_root.resolve(strict=False)
    resolved_path = (
        path.resolve(strict=False)
        if path.is_absolute()
        else (resolved_root / path).resolve(strict=False)
    )

    try:
        relative_path = resolved_path.relative_to(resolved_root)
        return relative_path.as_posix() or "."
    except ValueError:
        pass

    candidate_roots: list[Path] = []
    if resolved_path.is_dir():
        candidate_roots.append(resolved_path)
    candidate_roots.extend(resolved_path.parents)

    for candidate_root in candidate_roots:
        if not _same_location(candidate_root, resolved_root):
            continue
        relative_path = resolved_path.relative_to(candidate_root)
        return relative_path.as_posix() or "."

    return resolved_path.as_posix()
