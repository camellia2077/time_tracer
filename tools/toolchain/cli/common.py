import argparse
import shlex
from collections.abc import Iterable

from ..core.context import Context
from .model import ParserDefaults


def parse_cmake_args(raw_args: Iterable[str] | None) -> list[str]:
    flattened: list[str] = []
    for raw in raw_args or []:
        flattened.extend(shlex.split(raw, posix=False))
    return flattened


def apply_app_path_override(args: argparse.Namespace, ctx: Context) -> int:
    if getattr(args, "app_path", None):
        if not getattr(args, "app", None):
            print("Error: --app-path requires --app.")
            return 2
        ctx.set_app_path_override(args.app, args.app_path)
    return 0


def resolve_fixed_build_dir(ctx: Context, app_name: str | None) -> str | None:
    if not app_name:
        return None
    fixed_build_dir = (getattr(ctx.get_app_metadata(app_name), "fixed_build_dir", "") or "").strip()
    if fixed_build_dir:
        return fixed_build_dir
    return None


def reject_unsupported_build_dir_override(
    *,
    ctx: Context,
    app_name: str | None,
    build_dir_name: str | None,
    command_name: str,
) -> int:
    normalized_build_dir = (build_dir_name or "").strip()
    if not normalized_build_dir:
        return 0
    fixed_build_dir = resolve_fixed_build_dir(ctx, app_name)
    if not fixed_build_dir:
        return 0
    print(
        f"Error: `{command_name}` does not support `--build-dir` for app `{app_name}`. "
        f"This backend always uses `{fixed_build_dir}`."
    )
    return 2


def add_profile_arg(parser_obj: argparse.ArgumentParser, defaults: ParserDefaults) -> None:
    help_text = (
        "Build profile from tools/toolchain/config/*.toml."
    )
    if defaults.default_profile:
        help_text += f" Default profile: {defaults.default_profile}."
    if defaults.profile_choices:
        parser_obj.add_argument(
            "--profile",
            choices=defaults.profile_choices,
            default=None,
            help=help_text,
        )
    else:
        parser_obj.add_argument("--profile", default=None, help=help_text)


def add_source_scope_arg(
    parser_obj: argparse.ArgumentParser,
    defaults: ParserDefaults,
    *,
    help_suffix: str = "",
) -> None:
    help_text = "Named clang-tidy source scope preset from tools/toolchain/config/*.toml."
    if defaults.source_scope_choices:
        help_text += " Available: " + ", ".join(defaults.source_scope_choices) + "."
    if help_suffix:
        help_text += f" {help_suffix}"
    if defaults.source_scope_choices:
        parser_obj.add_argument(
            "--source-scope",
            choices=defaults.source_scope_choices,
            default=None,
            help=help_text,
        )
    else:
        parser_obj.add_argument("--source-scope", default=None, help=help_text)


def add_tidy_build_dir_arg(
    parser_obj: argparse.ArgumentParser,
    *,
    help_text: str = "Override tidy workspace directory name (default: build_tidy).",
) -> None:
    parser_obj.add_argument(
        "--tidy-build-dir",
        default=None,
        help=help_text,
    )


def add_task_selector_args(parser_obj: argparse.ArgumentParser) -> None:
    parser_obj.add_argument(
        "--task-log",
        default=None,
        help="Explicit task log path. Overrides --batch-id/--task-id selection.",
    )
    parser_obj.add_argument(
        "--batch-id",
        default=None,
        help="Batch identifier under tasks/ (e.g. 2, 002, batch_002).",
    )
    parser_obj.add_argument(
        "--task-id",
        default=None,
        help="Task identifier (e.g. 11, 011). When omitted, use the smallest pending task.",
    )
