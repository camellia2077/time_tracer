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


def add_profile_arg(parser_obj: argparse.ArgumentParser, defaults: ParserDefaults) -> None:
    help_text = "Build profile from scripts/toolchain/config.toml."
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
