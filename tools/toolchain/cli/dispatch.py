import argparse

from ..core.context import Context
from .common import apply_app_path_override, print_cli_error
from .registry import command_map


def dispatch_command(args: argparse.Namespace, ctx: Context) -> int:
    app_override_ret = apply_app_path_override(args, ctx)
    if app_override_ret != 0:
        return app_override_ret

    handler = command_map().get(args.command)
    if handler is None:
        print_cli_error(f"Error: unsupported command `{args.command}`")
        return 2
    return handler.run(args, ctx)
