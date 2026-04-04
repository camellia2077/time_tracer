from __future__ import annotations

from pathlib import Path

from ...core.context import Context

DEFAULT_CONFIG_FILE = ".clang-tidy"
STRICT_CONFIG_FILE = ".clang-tidy.strict"
CMAKE_CACHE_KEY_CONFIG_FILE = "TT_CLANG_TIDY_CONFIG_FILE"


def resolve_config_path(
    ctx: Context,
    *,
    config_file: str | None = None,
    strict_config: bool = False,
) -> Path:
    if config_file and strict_config:
        raise ValueError("`config_file` and `strict_config` are mutually exclusive.")
    selected = STRICT_CONFIG_FILE if strict_config else (config_file or DEFAULT_CONFIG_FILE)
    config_path = Path(selected)
    if not config_path.is_absolute():
        config_path = ctx.repo_root / config_path
    return config_path.resolve(strict=False)


def resolve_config_cache_value(
    ctx: Context,
    *,
    config_file: str | None = None,
    strict_config: bool = False,
) -> str:
    return str(
        resolve_config_path(
            ctx,
            config_file=config_file,
            strict_config=strict_config,
        )
    ).replace("\\", "/")


def build_config_file_args(
    ctx: Context,
    *,
    config_file: str | None = None,
    strict_config: bool = False,
) -> list[str]:
    resolved = resolve_config_path(
        ctx,
        config_file=config_file,
        strict_config=strict_config,
    )
    return [f"--config-file={resolved}"]


def build_cli_args(
    *,
    config_file: str | None = None,
    strict_config: bool = False,
) -> list[str]:
    if strict_config:
        return ["--strict-config"]
    normalized_config = str(config_file or "").strip()
    if normalized_config:
        return ["--config-file", normalized_config]
    return []
