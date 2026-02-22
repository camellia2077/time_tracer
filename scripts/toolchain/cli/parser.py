import argparse

from ..core.context import Context
from .model import ParserDefaults
from .registry import command_specs


def _add_app_argument(
    parser: argparse.ArgumentParser,
    app_choices: list[str],
    mode: str,
) -> None:
    if mode == "none":
        return
    required = mode == "required"
    if app_choices:
        parser.add_argument("--app", required=required, choices=app_choices)
    else:
        parser.add_argument("--app", required=required)


def build_parser(ctx: Context) -> argparse.ArgumentParser:
    defaults = ParserDefaults.from_context(ctx)
    parser = argparse.ArgumentParser(description="Unified build/test toolchain")
    subparsers = parser.add_subparsers(dest="command", required=True)

    for spec in command_specs():
        command_parser = subparsers.add_parser(spec.name, help=spec.help)
        _add_app_argument(command_parser, defaults.app_choices, spec.app_mode)
        if spec.add_app_path and spec.app_mode != "none":
            command_parser.add_argument("--app-path", help="Override default application path")
        spec.register(command_parser, defaults)

    return parser
