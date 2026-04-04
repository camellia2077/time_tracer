import argparse

from ....commands.tidy.fix import TidyFixCommand
from ....core.context import Context
from ...common import add_source_scope_arg, add_tidy_build_dir_arg, add_tidy_config_args
from ...model import CommandSpec, ParserDefaults


def register(parser: argparse.ArgumentParser, defaults: ParserDefaults) -> None:
    add_source_scope_arg(
        parser,
        defaults,
        help_suffix="Used to select a named scoped tidy source set.",
    )
    add_tidy_build_dir_arg(parser)
    add_tidy_config_args(parser)
    parser.add_argument(
        "--limit",
        type=int,
        default=None,
        help="0/omit = full tidy-fix target; N>0 = tidy_fix_step_N (prefix range).",
    )
    parser.add_argument(
        "--jobs",
        type=int,
        default=None,
        help="Ninja parallel jobs, e.g. 16",
    )
    tidy_fix_keep_going_group = parser.add_mutually_exclusive_group()
    tidy_fix_keep_going_group.add_argument(
        "--keep-going",
        dest="keep_going",
        action="store_true",
        default=None,
        help="Enable Ninja keep-going mode (-k 0) for tidy-fix build.",
    )
    tidy_fix_keep_going_group.add_argument(
        "--no-keep-going",
        dest="keep_going",
        action="store_false",
        help="Disable Ninja keep-going mode (-k 0) for tidy-fix build.",
    )


def run(args: argparse.Namespace, ctx: Context) -> int:
    cmd = TidyFixCommand(ctx)
    return cmd.execute(
        app_name=args.app,
        limit=args.limit,
        jobs=args.jobs,
        keep_going=args.keep_going,
        source_scope=args.source_scope,
        tidy_build_dir_name=args.tidy_build_dir,
        config_file=args.config_file,
        strict_config=bool(args.strict_config),
    )


COMMAND = CommandSpec(name="tidy-fix", register=register, run=run)
