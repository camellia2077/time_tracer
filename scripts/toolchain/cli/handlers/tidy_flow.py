import argparse

from ...commands.tidy.flow import TidyFlowCommand
from ...core.context import Context
from ..common import add_profile_arg
from ..model import CommandSpec, ParserDefaults


def register(parser: argparse.ArgumentParser, defaults: ParserDefaults) -> None:
    tidy_flow_target_group = parser.add_mutually_exclusive_group()
    tidy_flow_target_group.add_argument(
        "--n",
        type=int,
        default=None,
        help="Max tasks to process in this run (default: 1)",
    )
    tidy_flow_target_group.add_argument(
        "--all",
        action="store_true",
        help="Process tasks until none remain (or manual task blocks)",
    )
    parser.add_argument(
        "--resume",
        action="store_true",
        help="Reuse existing task logs when present",
    )
    parser.add_argument(
        "--test-every",
        type=int,
        default=3,
        help="Run verify after every K cleaned tasks inside tidy-loop",
    )
    parser.add_argument(
        "--concise",
        action="store_true",
        help="Use concise test output when supported",
    )
    parser.add_argument(
        "--jobs",
        type=int,
        default=None,
        help="Ninja parallel jobs for tidy generation, e.g. 16",
    )
    parser.add_argument(
        "--parse-workers",
        type=int,
        default=None,
        help="Parallel workers for tidy log splitting",
    )
    add_profile_arg(parser, defaults)
    parser.add_argument(
        "--build-dir",
        default=None,
        help="Override verify-stage build directory (default: build_fast or profile build_dir).",
    )
    tidy_flow_keep_going_group = parser.add_mutually_exclusive_group()
    tidy_flow_keep_going_group.add_argument(
        "--keep-going",
        dest="keep_going",
        action="store_true",
        default=None,
        help="Enable Ninja keep-going mode (-k 0) for tidy generation.",
    )
    tidy_flow_keep_going_group.add_argument(
        "--no-keep-going",
        dest="keep_going",
        action="store_false",
        help="Disable Ninja keep-going mode (-k 0) for tidy generation.",
    )
    tidy_flow_fix_group = parser.add_mutually_exclusive_group()
    tidy_flow_fix_group.add_argument(
        "--with-tidy-fix",
        dest="run_tidy_fix",
        action="store_true",
        default=None,
        help="Run a tidy-fix pass before tidy task generation.",
    )
    tidy_flow_fix_group.add_argument(
        "--no-tidy-fix",
        dest="run_tidy_fix",
        action="store_false",
        help="Skip tidy-fix pass before tidy task generation.",
    )
    parser.add_argument(
        "--tidy-fix-limit",
        type=int,
        default=None,
        help="0/omit = full tidy-fix target; N>0 = tidy_fix_step_N (prefix range).",
    )
    parser.add_argument(
        "--kill-build-procs",
        action="store_true",
        help="Kill cmake/ninja/ccache before configure/build stages (default: off)",
    )
    parser.add_argument(
        "--no-kill-build-procs",
        action="store_true",
        help=argparse.SUPPRESS,
    )


def run(args: argparse.Namespace, ctx: Context) -> int:
    kill_build_procs = bool(args.kill_build_procs and not args.no_kill_build_procs)
    effective_n = args.n if args.n is not None else 1
    cmd = TidyFlowCommand(ctx)
    return cmd.execute(
        app_name=args.app,
        process_all=args.all,
        n=effective_n,
        resume=args.resume,
        test_every=args.test_every,
        concise=args.concise,
        jobs=args.jobs,
        parse_workers=args.parse_workers,
        keep_going=args.keep_going,
        run_tidy_fix=args.run_tidy_fix,
        tidy_fix_limit=args.tidy_fix_limit,
        build_dir_name=args.build_dir,
        profile_name=args.profile,
        kill_build_procs=kill_build_procs,
    )


COMMAND = CommandSpec(name="tidy-flow", register=register, run=run)
