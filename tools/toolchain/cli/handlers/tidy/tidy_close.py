import argparse

from ....commands.tidy.execution.close import TidyCloseCommand
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
        "--dry-run",
        action="store_true",
        help="Preview the final-full tidy gate without changing tasks or running verify.",
    )
    parser.add_argument(
        "--jobs",
        type=int,
        default=None,
        help="Bounded parallel jobs for final full tidy (0 = auto-throttled).",
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
    if args.dry_run and args.tidy_only:
        print("Error: --dry-run cannot be combined with --tidy-only.")
        return 2
    cmd = TidyCloseCommand(ctx)
    return cmd.execute(
        app_name=args.app,
        keep_going=args.keep_going,
        verify_build_dir_name=args.build_dir,
        tidy_build_dir_name=args.tidy_build_dir,
        source_scope=args.source_scope,
        profile_name=args.profile,
        jobs=args.jobs,
        concise=args.concise,
        kill_build_procs=kill_build_procs,
        tidy_only=args.tidy_only,
        dry_run=args.dry_run,
        config_file=args.config_file,
        strict_config=strict_config,
    )


COMMAND = CommandSpec(name="tidy-close", register=register, run=run)
