import argparse

from ...commands.tidy.batch import TidyBatchCommand
from ...core.context import Context
from ..common import add_profile_arg
from ..model import CommandSpec, ParserDefaults


def register(parser: argparse.ArgumentParser, defaults: ParserDefaults) -> None:
    parser.add_argument(
        "--batch-id",
        required=True,
        help="Batch identifier under tasks/ (e.g. 1, 001, batch_001).",
    )
    parser.add_argument(
        "--preset",
        choices=["sop"],
        default=None,
        help=(
            "Apply a named preset. "
            "`sop` => --strict-clean --run-verify --concise --full-every 3 --keep-going."
        ),
    )
    parser.add_argument(
        "--strict-clean",
        action="store_true",
        help="Require latest verify result to be successful before clean.",
    )
    parser.add_argument(
        "--run-verify",
        action="store_true",
        help="Run verify before clean + refresh pipeline.",
    )
    parser.add_argument(
        "--full-every",
        type=int,
        default=None,
        help="Run full tidy every N newly-counted batches (<=0 disables cadence).",
    )
    parser.add_argument(
        "--build-dir",
        default=None,
        help=("Build directory for verify stage only (default: build_fast or profile build_dir)."),
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
    parser.add_argument(
        "--timeout-seconds",
        type=int,
        default=None,
        help=(
            "Soft timeout for tidy-batch execution. "
            "When reached, command writes a checkpoint and exits with code 124."
        ),
    )
    parser.add_argument(
        "--no-resume-checkpoint",
        action="store_true",
        help="Disable automatic resume from tidy_batch_checkpoint state.",
    )
    tidy_batch_keep_going_group = parser.add_mutually_exclusive_group()
    tidy_batch_keep_going_group.add_argument(
        "--keep-going",
        dest="keep_going",
        action="store_true",
        default=None,
        help="Continue incremental chunks after a non-zero clang-tidy exit.",
    )
    tidy_batch_keep_going_group.add_argument(
        "--no-keep-going",
        dest="keep_going",
        action="store_false",
        help="Stop incremental refresh on first non-zero clang-tidy exit.",
    )


def _apply_preset(args: argparse.Namespace) -> None:
    if args.preset != "sop":
        return
    args.strict_clean = True
    args.run_verify = True
    args.concise = True
    if args.full_every is None:
        args.full_every = 3
    if args.keep_going is None:
        args.keep_going = True


def run(args: argparse.Namespace, ctx: Context) -> int:
    _apply_preset(args)
    full_every = 3 if args.full_every is None else args.full_every
    kill_build_procs = bool(args.kill_build_procs and not args.no_kill_build_procs)
    cmd = TidyBatchCommand(ctx)
    return cmd.execute(
        app_name=args.app,
        batch_id=args.batch_id,
        strict_clean=args.strict_clean,
        full_every=full_every,
        keep_going=args.keep_going,
        run_verify=args.run_verify,
        verify_build_dir_name=args.build_dir,
        profile_name=args.profile,
        concise=args.concise,
        kill_build_procs=kill_build_procs,
        timeout_seconds=args.timeout_seconds,
        resume_checkpoint=not args.no_resume_checkpoint,
    )


COMMAND = CommandSpec(name="tidy-batch", register=register, run=run)
