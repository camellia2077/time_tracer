import argparse

from ....commands.tidy.step import TidyStepCommand
from ....core.context import Context
from ...common import (
    add_profile_arg,
    add_source_scope_arg,
    add_task_selector_args,
    add_tidy_build_dir_arg,
)
from ...model import CommandSpec, ParserDefaults


def register(parser: argparse.ArgumentParser, defaults: ParserDefaults) -> None:
    add_source_scope_arg(
        parser,
        defaults,
        help_suffix="Used to resolve scoped tidy source roots during step execution.",
    )
    add_tidy_build_dir_arg(parser)
    add_task_selector_args(parser)
    add_profile_arg(parser, defaults)
    parser.add_argument(
        "--build-dir",
        default=None,
        help="Override verify-stage build directory (default: build_fast or profile build_dir).",
    )
    parser.add_argument(
        "--concise",
        action="store_true",
        help="Use concise verify output when supported.",
    )
    parser.add_argument(
        "--kill-build-procs",
        action="store_true",
        help="Kill cmake/ninja/ccache before verify build stages (default: off).",
    )
    parser.add_argument(
        "--no-kill-build-procs",
        action="store_true",
        help=argparse.SUPPRESS,
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Preview one task step without modifying sources or running verify.",
    )
    parser.add_argument(
        "--strict",
        action="store_true",
        help="Return non-zero when task auto-fix encounters failures.",
    )


def run(args: argparse.Namespace, ctx: Context) -> int:
    kill_build_procs = bool(args.kill_build_procs and not args.no_kill_build_procs)
    cmd = TidyStepCommand(ctx)
    return cmd.execute(
        app_name=args.app,
        task_log_path=args.task_log,
        batch_id=args.batch_id,
        task_id=args.task_id,
        tidy_build_dir_name=args.tidy_build_dir,
        source_scope=args.source_scope,
        verify_build_dir_name=args.build_dir,
        profile_name=args.profile,
        concise=args.concise,
        kill_build_procs=kill_build_procs,
        dry_run=args.dry_run,
        strict=args.strict,
    )


COMMAND = CommandSpec(name="tidy-step", register=register, run=run)
