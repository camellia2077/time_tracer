import argparse

from ....commands.cmd_quality.refresh_golden import RefreshGoldenCommand
from ....core.context import Context
from ...common import add_profile_arg
from ...model import CommandSpec, ParserDefaults


def register(parser: argparse.ArgumentParser, defaults: ParserDefaults) -> None:
    add_profile_arg(parser, defaults)
    parser.add_argument(
        "--build-dir",
        default=None,
        help="Build directory used by rust_cli runtime and verify stage (default: build_fast).",
    )
    parser.add_argument(
        "--concise",
        action="store_true",
        help="Use concise output for verify stage when not skipped.",
    )
    parser.add_argument(
        "--skip-verify",
        action="store_true",
        help="Skip pre-refresh artifact verify and only refresh snapshots from existing outputs.",
    )
    parser.add_argument(
        "--recent-range",
        default=None,
        help=(
            "Optional fixed recent range (YYYY-MM-DD|YYYY-MM-DD). "
            "When set, refresh-golden updates gate_cases.toml automatically."
        ),
    )
    parser.add_argument(
        "--range-argument",
        default=None,
        help=(
            "Optional fixed range argument (YYYY-MM-DD|YYYY-MM-DD). "
            "When set, refresh-golden updates gate_cases.toml automatically."
        ),
    )


def run(args: argparse.Namespace, ctx: Context) -> int:
    cmd = RefreshGoldenCommand(ctx)
    return cmd.execute(
        app_name=args.app,
        build_dir_name=args.build_dir,
        profile_name=args.profile,
        concise=bool(args.concise),
        skip_verify=bool(args.skip_verify),
        recent_range=args.recent_range,
        range_argument=args.range_argument,
    )


COMMAND = CommandSpec(name="refresh-golden", register=register, run=run)
