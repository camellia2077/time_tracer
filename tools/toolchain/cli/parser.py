import argparse
import sys

from ..core.context import Context
from .model import ParserDefaults
from .registry import command_specs


class ToolchainArgumentParser(argparse.ArgumentParser):
    def error(self, message: str) -> None:
        if "unrecognized arguments:" in message:
            argv = sys.argv[1:]
            command = argv[0] if argv else ""
            hint_lines: list[str] = []

            if command:
                hint_lines.append(f"Hint: run `python tools/run.py {command} -h` to inspect supported flags.")
            else:
                hint_lines.append("Hint: run `python tools/run.py -h` to inspect supported commands.")

            cmake_flag_requested = "--cmake-args" in message or any(
                arg.startswith("--cmake-args") for arg in argv
            )
            if cmake_flag_requested:
                hint_lines.append(
                    "Hint: for CMake argument passthrough, use `build`/`verify` "
                    "or `post-change --cmake-args=...`."
                )
            else:
                hint_lines.append(
                    "Hint: if you are trying a build validation flow, consider "
                    "`python tools/run.py build ...` or `python tools/run.py verify ...`."
                )

            message = f"{message}\n" + "\n".join(hint_lines)
        super().error(message)


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
    parser = ToolchainArgumentParser(description="Unified build/test toolchain")
    subparsers = parser.add_subparsers(dest="command", required=True)

    for spec in command_specs():
        command_parser = subparsers.add_parser(spec.name, help=spec.help)
        _add_app_argument(command_parser, defaults.app_choices, spec.app_mode)
        if spec.add_app_path and spec.app_mode != "none":
            command_parser.add_argument("--app-path", help="Override default application path")
        spec.register(command_parser, defaults)

    return parser
