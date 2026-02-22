from __future__ import annotations

import subprocess
from pathlib import Path


def collect_changed_files(repo_root: Path) -> list[str]:
    changed: set[str] = set()
    changed.update(_run_git_name_list(repo_root, ["diff", "--name-only", "--relative"]))
    changed.update(_run_git_name_list(repo_root, ["diff", "--cached", "--name-only", "--relative"]))
    changed.update(_run_git_name_list(repo_root, ["ls-files", "--others", "--exclude-standard"]))
    return sorted(changed)


def _run_git_name_list(repo_root: Path, git_args: list[str]) -> list[str]:
    try:
        completed = subprocess.run(
            ["git", *git_args],
            cwd=str(repo_root),
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace",
            check=False,
        )
    except FileNotFoundError:
        return []

    if completed.returncode != 0:
        return []

    paths: list[str] = []
    for raw_line in completed.stdout.splitlines():
        normalized = _normalize_git_path(raw_line)
        if normalized:
            paths.append(normalized)
    return paths


def _normalize_git_path(raw_path: str) -> str:
    text = raw_path.strip().strip('"')
    if not text:
        return ""
    if " -> " in text:
        text = text.split(" -> ", 1)[1]
    return text.replace("\\", "/")
