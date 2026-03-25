#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
from pathlib import Path
import subprocess

from tools.toolchain.commands.cmd_quality.verify_internal.verify_profile_inference import (
    classify_changed_paths,
)


_CODE_PATH_PREFIXES = (
    "apps/",
    "libs/",
    "tools/",
    "test/",
    "assets/",
    "cmake/",
    ".github/workflows/",
)
_CODE_FILES = {
    ".gitignore",
    ".gitattributes",
    ".clang-format",
    ".clang-tidy",
    "pyproject.toml",
}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Compute focused capability smoke profiles for CI."
    )
    parser.add_argument("--repo-root", default=".")
    parser.add_argument("--event-name", default="")
    parser.add_argument("--base-sha", default="")
    parser.add_argument("--head-sha", default="HEAD")
    return parser.parse_args()


def _run_git(repo_root: Path, args: list[str]) -> list[str]:
    completed = subprocess.run(
        ["git", *args],
        cwd=repo_root,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        encoding="utf-8",
        errors="replace",
        check=False,
    )
    if completed.returncode != 0:
        return []
    return [line.strip().replace("\\", "/") for line in completed.stdout.splitlines() if line.strip()]


def _is_code_path(path: str) -> bool:
    normalized = path.strip().replace("\\", "/")
    return normalized in _CODE_FILES or normalized.startswith(_CODE_PATH_PREFIXES)


def _build_profile_matrix(profiles: tuple[str, ...]) -> str:
    return json.dumps({"include": [{"profile": profile} for profile in profiles]})


def build_payload(event_name: str, changed_paths: list[str]) -> dict[str, object]:
    normalized_event = (event_name or "").strip().lower()
    if normalized_event in {"workflow_dispatch", "schedule"}:
        return {
            "run_heavy_verify": True,
            "run_capability_smoke": False,
            "code_changes": True,
            "mode": "heavy_only",
            "reason": f"{normalized_event} always runs heavy verify",
            "profiles": [],
            "matrix": _build_profile_matrix(()),
        }

    code_paths = [path for path in changed_paths if _is_code_path(path)]
    if not code_paths:
        return {
            "run_heavy_verify": False,
            "run_capability_smoke": False,
            "code_changes": False,
            "mode": "skip",
            "reason": "no code paths changed",
            "profiles": [],
            "matrix": _build_profile_matrix(()),
        }

    inference = classify_changed_paths(code_paths)
    return {
        "run_heavy_verify": True,
        "run_capability_smoke": not inference.fallback_to_fast,
        "code_changes": True,
        "mode": "fast_fallback" if inference.fallback_to_fast else "focused",
        "reason": inference.reason,
        "profiles": list(inference.profiles if not inference.fallback_to_fast else ()),
        "matrix": _build_profile_matrix(
            inference.profiles if not inference.fallback_to_fast else ()
        ),
    }


def main() -> int:
    args = parse_args()
    repo_root = Path(args.repo_root).resolve()
    event_name = (args.event_name or "").strip().lower()

    diff_range = f"{args.base_sha}...{args.head_sha}" if args.base_sha else ""
    changed_paths = (
        _run_git(repo_root, ["diff", "--name-only", diff_range]) if diff_range else []
    )
    if not changed_paths:
        changed_paths = _run_git(repo_root, ["diff", "--name-only", "HEAD~1...HEAD"])
    payload = build_payload(event_name, changed_paths)
    print(json.dumps(payload))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
