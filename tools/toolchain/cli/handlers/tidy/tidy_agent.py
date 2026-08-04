import argparse

from ....commands.tidy.execution.agent_run import TidyAgentRunCommand
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
        "--max-clusters",
        type=int,
        default=3,
        help="Maximum source clusters per bounded run (default: 3).",
    )
    parser.add_argument(
        "--max-tasks",
        type=int,
        default=10,
        help="Maximum pending tasks per bounded run (default: 10).",
    )
    parser.add_argument(
        "--max-minutes",
        type=int,
        default=30,
        help="Maximum wall-clock minutes per bounded run (default: 30).",
    )
    parser.add_argument("--build-dir", default=None)
    parser.add_argument("--concise", action="store_true")
    parser.add_argument("--kill-build-procs", action="store_true")
    parser.add_argument("--no-kill-build-procs", action="store_true", help=argparse.SUPPRESS)
    add_tidy_build_dir_arg(parser)
    add_source_scope_arg(parser, defaults, help_suffix="Used to resolve the current tidy source queue.")
    add_tidy_config_args(parser)
    add_profile_arg(parser, defaults)
    parser.add_argument("--strict", action="store_true")
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Preview one current source cluster without modifying sources or archiving tasks.",
    )


def run(args: argparse.Namespace, ctx: Context) -> int:
    return TidyAgentRunCommand(ctx).execute(
        app_name=args.app,
        source_scope=args.source_scope,
        tidy_build_dir_name=args.tidy_build_dir,
        max_clusters=args.max_clusters,
        max_tasks=args.max_tasks,
        max_minutes=args.max_minutes,
        verify_build_dir_name=args.build_dir,
        profile_name=args.profile,
        concise=args.concise,
        kill_build_procs=bool(args.kill_build_procs and not args.no_kill_build_procs),
        strict=args.strict,
        dry_run=args.dry_run,
        config_file=args.config_file,
        strict_config=bool(args.strict_config),
    )


COMMAND = CommandSpec(name="tidy-agent", register=register, run=run)
