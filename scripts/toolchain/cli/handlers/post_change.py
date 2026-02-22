import argparse

from ...commands.cmd_workflow.post_change import PostChangeCommand
from ...core.context import Context
from ..common import add_profile_arg
from ..model import CommandSpec, ParserDefaults


def register(parser: argparse.ArgumentParser, defaults: ParserDefaults) -> None:
    add_profile_arg(parser, defaults)
    parser.add_argument(
        "--build-dir",
        default=None,
        help=(
            "Build directory for post-change flow "
            f"(default: {defaults.post_change_default_build_dir}, "
            "or profile build_dir when --profile is set)."
        ),
    )
    parser.add_argument(
        "--run-tests",
        choices=["auto", "always", "never"],
        default=defaults.post_change_default_run_tests,
        help="Test policy for post-change flow.",
    )
    parser.add_argument(
        "--script-changes",
        choices=["auto", "build", "skip"],
        default=defaults.post_change_default_script_changes,
        help="Build policy when only script files changed.",
    )
    parser.add_argument(
        "--concise",
        action="store_true",
        help="Use concise output for test runner when tests are executed.",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Show post-change decision without running configure/build/test.",
    )
    parser.add_argument(
        "--kill-build-procs",
        action="store_true",
        help="Kill cmake/ninja/ccache before post-change build stages (default: off)",
    )
    parser.add_argument(
        "--no-kill-build-procs",
        action="store_true",
        help=argparse.SUPPRESS,
    )


def run(args: argparse.Namespace, ctx: Context) -> int:
    kill_build_procs = bool(args.kill_build_procs and not args.no_kill_build_procs)
    effective_post_change_build_dir = args.build_dir

    default_build_dir = (
        ctx.config.post_change.default_build_dir.strip()
        if ctx.config.post_change.default_build_dir
        else "build_fast"
    )
    if not default_build_dir:
        default_build_dir = "build_fast"

    if not effective_post_change_build_dir and not args.profile:
        should_apply_default_build_dir = True
        if args.app:
            backend = (
                (getattr(ctx.get_app_metadata(args.app), "backend", "cmake") or "cmake")
                .strip()
                .lower()
            )
            if backend == "gradle":
                should_apply_default_build_dir = False
        if should_apply_default_build_dir:
            effective_post_change_build_dir = default_build_dir

    cmd = PostChangeCommand(ctx)
    return cmd.execute(
        app_name=args.app,
        build_dir_name=effective_post_change_build_dir,
        profile_name=args.profile,
        run_tests=args.run_tests,
        script_changes=args.script_changes,
        concise=args.concise,
        dry_run=args.dry_run,
        kill_build_procs=kill_build_procs,
    )


COMMAND = CommandSpec(
    name="post-change",
    register=register,
    run=run,
    app_mode="optional",
    add_app_path=True,
)
