import argparse

from ...commands.tidy.refresh import TidyRefreshCommand
from ...core.context import Context
from ..model import CommandSpec, ParserDefaults


def register(parser: argparse.ArgumentParser, _: ParserDefaults) -> None:
    parser.add_argument(
        "--batch-id",
        default=None,
        help="Batch identifier from tasks_done (e.g. 1, 001, batch_001).",
    )
    parser.add_argument(
        "--full-every",
        type=int,
        default=3,
        help="Run full tidy every N newly-counted batches (<=0 disables cadence).",
    )
    parser.add_argument(
        "--neighbor-scope",
        choices=["none", "dir", "module"],
        default="none",
        help="Incremental expansion scope beyond touched files.",
    )
    parser.add_argument(
        "--jobs",
        type=int,
        default=None,
        help="Ninja jobs for full tidy when triggered.",
    )
    parser.add_argument(
        "--parse-workers",
        type=int,
        default=None,
        help="Log split workers for full tidy when triggered.",
    )
    parser.add_argument(
        "--build-dir",
        default=None,
        help="Override tidy build directory name (default: build_tidy).",
    )
    tidy_refresh_keep_going_group = parser.add_mutually_exclusive_group()
    tidy_refresh_keep_going_group.add_argument(
        "--keep-going",
        dest="keep_going",
        action="store_true",
        default=None,
        help="Continue incremental chunks after a non-zero clang-tidy exit.",
    )
    tidy_refresh_keep_going_group.add_argument(
        "--no-keep-going",
        dest="keep_going",
        action="store_false",
        help="Stop incremental refresh on first non-zero clang-tidy exit.",
    )
    parser.add_argument(
        "--force-full",
        action="store_true",
        help="Run a full tidy immediately after incremental refresh.",
    )
    parser.add_argument(
        "--final-full",
        action="store_true",
        help="Run a final full tidy pass (can be used without --batch-id).",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Preview selected files/cadence decision without running tidy.",
    )


def run(args: argparse.Namespace, ctx: Context) -> int:
    cmd = TidyRefreshCommand(ctx)
    return cmd.execute(
        app_name=args.app,
        batch_id=args.batch_id,
        full_every=args.full_every,
        neighbor_scope=args.neighbor_scope,
        jobs=args.jobs,
        parse_workers=args.parse_workers,
        keep_going=args.keep_going,
        force_full=args.force_full,
        final_full=args.final_full,
        build_dir_name=args.build_dir,
        dry_run=args.dry_run,
    )


COMMAND = CommandSpec(name="tidy-refresh", register=register, run=run)
