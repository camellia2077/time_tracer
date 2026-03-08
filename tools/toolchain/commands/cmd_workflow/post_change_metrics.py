from __future__ import annotations

from pathlib import Path

from ...core.context import Context
from ...services.suite_registry import resolve_suite_build_app

_BINARY_SUFFIXES = {".exe", ".dll", ".so", ".dylib", ".a", ".lib"}


def _to_repo_relative(repo_root: Path, path: Path) -> str:
    try:
        return str(path.resolve().relative_to(repo_root.resolve())).replace("\\", "/")
    except ValueError:
        return str(path.resolve())


def _resolve_candidate_bin_dirs(ctx: Context, app_name: str, build_dir_name: str) -> list[Path]:
    candidates: list[Path] = []

    primary_app_dir = ctx.get_app_dir(app_name)
    candidates.append(primary_app_dir / build_dir_name / "bin")

    suite_build_app = resolve_suite_build_app(app_name)
    if suite_build_app and suite_build_app != app_name:
        suite_app_dir = ctx.get_app_dir(suite_build_app)
        candidates.append(suite_app_dir / build_dir_name / "bin")

    deduped: list[Path] = []
    seen: set[Path] = set()
    for candidate in candidates:
        resolved_candidate = candidate.resolve()
        if resolved_candidate in seen:
            continue
        seen.add(resolved_candidate)
        deduped.append(candidate)
    return deduped


def collect_binary_size_metrics(ctx: Context, app_name: str, build_dir_name: str) -> dict:
    candidate_dirs = _resolve_candidate_bin_dirs(ctx, app_name, build_dir_name)
    candidate_items = [
        {
            "path": _to_repo_relative(ctx.repo_root, candidate),
            "exists": candidate.exists(),
        }
        for candidate in candidate_dirs
    ]

    artifacts: list[dict] = []
    for candidate in candidate_dirs:
        if not candidate.exists():
            continue
        for file_path in candidate.rglob("*"):
            if not file_path.is_file():
                continue
            if file_path.suffix.lower() not in _BINARY_SUFFIXES:
                continue
            try:
                size_bytes = file_path.stat().st_size
            except OSError:
                continue
            artifacts.append(
                {
                    "path": _to_repo_relative(ctx.repo_root, file_path),
                    "size_bytes": int(size_bytes),
                }
            )

    artifacts.sort(key=lambda item: (-int(item["size_bytes"]), str(item["path"])))
    total_bytes = int(sum(int(item["size_bytes"]) for item in artifacts))
    return {
        "candidate_dirs": candidate_items,
        "artifacts": artifacts,
        "count": len(artifacts),
        "total_bytes": total_bytes,
    }
