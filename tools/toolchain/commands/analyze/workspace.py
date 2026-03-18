from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path

from ...core.context import Context
from ..tidy import workspace as tidy_workspace

DEFAULT_ANALYZE_BUILD_DIR_NAME = "build_analyze"


@dataclass(frozen=True, slots=True)
class ResolvedAnalyzeWorkspace:
    source_scope: str | None
    build_dir_name: str
    source_roots: list[Path]
    prebuild_targets: list[str]


def resolve_analyze_build_dir_name(
    ctx: Context,
    *,
    build_dir_name: str | None = None,
    source_scope: str | None = None,
) -> str:
    explicit = (build_dir_name or "").strip()
    if explicit:
        return explicit
    normalized_scope = tidy_workspace.normalize_source_scope(source_scope)
    if normalized_scope:
        return f"{DEFAULT_ANALYZE_BUILD_DIR_NAME}_{normalized_scope}"
    return DEFAULT_ANALYZE_BUILD_DIR_NAME


def resolve_workspace(
    ctx: Context,
    *,
    build_dir_name: str | None = None,
    source_scope: str | None = None,
) -> ResolvedAnalyzeWorkspace:
    normalized_scope = tidy_workspace.normalize_source_scope(source_scope)
    return ResolvedAnalyzeWorkspace(
        source_scope=normalized_scope,
        build_dir_name=resolve_analyze_build_dir_name(
            ctx,
            build_dir_name=build_dir_name,
            source_scope=normalized_scope,
        ),
        source_roots=tidy_workspace.resolve_source_roots(ctx, normalized_scope),
        prebuild_targets=tidy_workspace.resolve_prebuild_targets(ctx, normalized_scope),
    )
