import argparse

from ....commands.tidy.close import TidyCloseCommand
from ....core.context import Context
from ...common import (
    add_profile_arg,
    add_source_scope_arg,
    add_tidy_build_dir_arg,
    add_tidy_config_args,
)
from ...model import CommandSpec, ParserDefaults


def register(parser: argparse.ArgumentParser, defaults: ParserDefaults) -> None:
    parser.add_argument(
        "--build-dir",
        default=None,
        help=(
            "Build directory for final verify stage only "
            "(default: build_fast or profile build_dir)."
        ),
    )
    add_tidy_build_dir_arg(parser)
    add_source_scope_arg(
        parser,
        defaults,
        help_suffix="Used when tidy-close triggers a final full tidy on a scoped workspace.",
    )
    add_tidy_config_args(parser)
    parser.add_argument(
        "--tidy-only",
        action="store_true",
        help="Close tidy queue only (skip verify gate).",
    )
    parser.add_argument(
        "--stabilize",
        action="store_true",
        help=(
            "If final full tidy repopulates tasks/, keep draining the refreshed "
            "queue and rerun final-full until it stabilizes or a task blocks progress."
        ),
    )
    parser.add_argument(
        "--jobs",
        type=int,
        default=None,
        help="Bounded parallel jobs for final full tidy (0 = auto-throttled).",
    )
    parser.add_argument(
        "--parse-workers",
        type=int,
        default=None,
        help="Log split workers for final full tidy when a full rebuild runs.",
    )
    add_profile_arg(parser, defaults)
    parser.add_argument(
        "--concise",
        action="store_true",
        help="Use concise output for verify test runner.",
    )
    parser.add_argument(
        "--kill-build-procs",
        action="store_true",
        help="Kill cmake/ninja/ccache before verify build stages (default: off)",
    )
    parser.add_argument(
        "--no-kill-build-procs",
        action="store_true",
        help=argparse.SUPPRESS,
    )
    tidy_close_keep_going_group = parser.add_mutually_exclusive_group()
    tidy_close_keep_going_group.add_argument(
        "--keep-going",
        dest="keep_going",
        action="store_true",
        default=None,
        help="Continue incremental chunks after a non-zero clang-tidy exit.",
    )
    tidy_close_keep_going_group.add_argument(
        "--no-keep-going",
        dest="keep_going",
        action="store_false",
        help="Stop incremental refresh on first non-zero clang-tidy exit.",
    )


def run(args: argparse.Namespace, ctx: Context) -> int:
    kill_build_procs = bool(args.kill_build_procs and not args.no_kill_build_procs)
    strict_config = bool(args.strict_config)
    stabilize = bool(args.stabilize)
    # In strict mode, tidy-close is treated as the final acceptance gate.
    # A final full tidy refresh can legitimately repopulate tasks/ with a new
    # queue wave, so we automatically enable stabilize to drain/retry instead
    # of failing early with a transient "pending_tasks" status.
    if strict_config:
        stabilize = True
    cmd = TidyCloseCommand(ctx)
    return cmd.execute(
        app_name=args.app,
        keep_going=args.keep_going,
        verify_build_dir_name=args.build_dir,
        tidy_build_dir_name=args.tidy_build_dir,
        source_scope=args.source_scope,
        profile_name=args.profile,
        jobs=args.jobs,
        parse_workers=args.parse_workers,
        concise=args.concise,
        kill_build_procs=kill_build_procs,
        tidy_only=args.tidy_only,
        config_file=args.config_file,
        strict_config=strict_config,
        stabilize=stabilize,
    )


COMMAND = CommandSpec(name="tidy-close", register=register, run=run)
