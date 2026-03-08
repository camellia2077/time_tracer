from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path

from ...core.context import Context

DEFAULT_TIDY_BUILD_DIR_NAME = "build_tidy"
CMAKE_CACHE_KEY_SOURCE_SCOPE = "TT_CLANG_TIDY_SOURCE_SCOPE"
CMAKE_CACHE_KEY_SOURCE_ROOTS = "TT_CLANG_TIDY_SOURCE_ROOTS"


@dataclass(frozen=True, slots=True)
class ResolvedTidyWorkspace:
    source_scope: str | None
    build_dir_name: str
    source_roots: list[Path]


def normalize_source_scope(source_scope: str | None) -> str | None:
    normalized = (source_scope or "").strip()
    return normalized or None


def resolve_source_scope_config(ctx: Context, source_scope: str | None):
    normalized = normalize_source_scope(source_scope)
    if normalized is None:
        return None
    scope_cfg = ctx.config.tidy.source_scopes.get(normalized)
    if scope_cfg is None:
        raise ValueError(f"unknown clang-tidy source scope `{normalized}`")
    return scope_cfg


def resolve_source_roots(ctx: Context, source_scope: str | None) -> list[Path]:
    scope_cfg = resolve_source_scope_config(ctx, source_scope)
    if scope_cfg is None:
        return []

    resolved: list[Path] = []
    seen: set[str] = set()
    for raw_root in scope_cfg.roots:
        text = str(raw_root).strip()
        if not text:
            continue
        root_path = Path(text)
        if not root_path.is_absolute():
            root_path = ctx.repo_root / root_path
        normalized = str(root_path.resolve()).replace("\\", "/")
        if normalized in seen:
            continue
        seen.add(normalized)
        resolved.append(Path(normalized))
    return resolved


def resolve_tidy_build_dir_name(
    ctx: Context,
    *,
    build_dir_name: str | None = None,
    source_scope: str | None = None,
) -> str:
    explicit = (build_dir_name or "").strip()
    if explicit:
        return explicit
    scope_cfg = resolve_source_scope_config(ctx, source_scope)
    scoped_build_dir = (getattr(scope_cfg, "tidy_build_dir", "") or "").strip() if scope_cfg else ""
    if scoped_build_dir:
        return scoped_build_dir
    return DEFAULT_TIDY_BUILD_DIR_NAME


def resolve_workspace(
    ctx: Context,
    *,
    build_dir_name: str | None = None,
    source_scope: str | None = None,
) -> ResolvedTidyWorkspace:
    normalized_scope = normalize_source_scope(source_scope)
    return ResolvedTidyWorkspace(
        source_scope=normalized_scope,
        build_dir_name=resolve_tidy_build_dir_name(
            ctx,
            build_dir_name=build_dir_name,
            source_scope=normalized_scope,
        ),
        source_roots=resolve_source_roots(ctx, normalized_scope),
    )


def source_scope_cache_values(ctx: Context, source_scope: str | None) -> tuple[str, str]:
    workspace = resolve_workspace(ctx, source_scope=source_scope)
    scope_value = workspace.source_scope or ""
    roots_value = ";".join(str(path).replace("\\", "/") for path in workspace.source_roots)
    return scope_value, roots_value


def build_source_scope_cmake_args(ctx: Context, source_scope: str | None) -> list[str]:
    scope_value, roots_value = source_scope_cache_values(ctx, source_scope)
    return [
        "-D",
        f"{CMAKE_CACHE_KEY_SOURCE_SCOPE}={scope_value}",
        "-D",
        f"{CMAKE_CACHE_KEY_SOURCE_ROOTS}={roots_value}",
    ]


def normalize_cache_roots_value(value: str | None) -> str:
    raw = (value or "").strip()
    if not raw:
        return ""
    normalized: list[str] = []
    for item in raw.split(";"):
        text = item.strip()
        if not text:
            continue
        normalized.append(text.replace("\\", "/"))
    return ";".join(normalized)
