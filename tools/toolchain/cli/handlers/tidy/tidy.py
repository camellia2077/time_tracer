import argparse

from ....commands.tidy import TidyCommand
from ....commands.shared.result_reporting import print_failure_report
from ....core.context import Context
from ...common import (
    add_build_dir_arg,
    add_concise_arg,
    add_kill_build_procs_args,
    add_profile_arg,
    reject_unsupported_build_dir_override,
    add_source_scope_arg,
    add_tidy_config_args,
    add_tidy_task_view_arg,
    append_tidy_config_to_command,
)
from ...model import CommandSpec, ParserDefaults
from ....commands.tidy import workspace as tidy_workspace


def _build_command_text(args: argparse.Namespace) -> str:
    parts = ["python tools/run.py tidy", "--app", args.app]
    if args.profile:
        parts.extend(["--profile", args.profile])
    if args.build_dir:
        parts.extend(["--build-dir", args.build_dir])
    if args.kill_build_procs and not args.no_kill_build_procs:
        parts.append("--kill-build-procs")
    if args.concise:
        parts.append("--concise")
    if args.jobs is not None:
        parts.extend(["--jobs", str(args.jobs)])
    if args.parse_workers is not None:
        parts.extend(["--parse-workers", str(args.parse_workers)])
    if args.source_scope:
        parts.extend(["--source-scope", args.source_scope])
    if args.task_view:
        parts.extend(["--task-view", args.task_view])
    append_tidy_config_to_command(
        parts,
        config_file=args.config_file,
        strict_config=bool(args.strict_config),
    )
    if args.keep_going is True:
        parts.append("--keep-going")
    elif args.keep_going is False:
        parts.append("--no-keep-going")
    for extra_arg in getattr(args, "extra_args", []) or []:
        if extra_arg == "--":
            continue
        parts.append(extra_arg)
    return " ".join(parts)


def register(parser: argparse.ArgumentParser, defaults: ParserDefaults) -> None:
    add_profile_arg(parser, defaults)
    add_build_dir_arg(
        parser,
        help_text="Override build directory name (default: build_tidy).",
    )
    add_kill_build_procs_args(parser)
    add_concise_arg(parser)
    parser.add_argument("--jobs", type=int, default=None, help="Ninja parallel jobs, e.g. 16")
    parser.add_argument(
        "--parse-workers",
        type=int,
        default=None,
        help="Parallel workers for log splitting",
    )
    add_source_scope_arg(
        parser,
        defaults,
        help_suffix="When omitted, use the app's default full tidy source set.",
    )
    add_tidy_config_args(parser)
    add_tidy_task_view_arg(parser)
    tidy_keep_going_group = parser.add_mutually_exclusive_group()
    tidy_keep_going_group.add_argument(
        "--keep-going",
        dest="keep_going",
        action="store_true",
        default=None,
        help="Enable Ninja keep-going mode (-k 0) for tidy build.",
    )
    tidy_keep_going_group.add_argument(
        "--no-keep-going",
        dest="keep_going",
        action="store_false",
        help="Disable Ninja keep-going mode (-k 0) for tidy build.",
    )
    parser.add_argument("extra_args", nargs=argparse.REMAINDER)


def run(args: argparse.Namespace, ctx: Context) -> int:
    build_dir_error = reject_unsupported_build_dir_override(
        ctx=ctx,
        app_name=args.app,
        build_dir_name=args.build_dir,
        command_name="tidy",
    )
    if build_dir_error != 0:
        return build_dir_error
    cmd = TidyCommand(ctx)
    ret = cmd.execute(
        args.app,
        args.extra_args,
        jobs=args.jobs,
        parse_workers=args.parse_workers,
        concise=bool(args.concise),
        profile_name=args.profile,
        kill_build_procs=bool(args.kill_build_procs and not args.no_kill_build_procs),
        keep_going=args.keep_going,
        source_scope=args.source_scope,
        build_dir_name=args.build_dir,
        task_view=args.task_view,
        config_file=args.config_file,
        strict_config=bool(args.strict_config),
    )
    if ret != 0:
        workspace = tidy_workspace.resolve_workspace(
            ctx,
            build_dir_name=args.build_dir,
            source_scope=args.source_scope,
        )
        print_failure_report(
            command=_build_command_text(args),
            exit_code=int(ret),
            next_action=f"Fix errors and rerun: {_build_command_text(args)}",
            app_name=args.app,
            repo_root=ctx.repo_root,
            stage="tidy",
            build_log_path=ctx.get_tidy_dir(args.app, workspace.build_dir_name) / "build.log",
            fallback_key_error_hint="Tidy failed. See command output above.",
            include_result_json=False,
        )
    return ret


COMMAND = CommandSpec(name="tidy", register=register, run=run)
