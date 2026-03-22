import argparse
import sys

from ...commands.cmd_build import BuildCommand
from ...commands.shared.result_reporting import print_failure_report
from ...core.context import Context
from ..common import (
    add_build_dir_arg,
    add_concise_arg,
    add_kill_build_procs_args,
    add_profile_arg,
    parse_cmake_args,
    reject_unsupported_build_dir_override,
)
from ..model import CommandSpec, ParserDefaults


def _build_command_text(args: argparse.Namespace) -> str:
    parts = ["python tools/run.py build", "--app", args.app]
    if args.tidy:
        parts.append("--tidy")
    if args.profile:
        parts.extend(["--profile", args.profile])
    if args.build_dir:
        parts.extend(["--build-dir", args.build_dir])
    if args.kill_build_procs and not args.no_kill_build_procs:
        parts.append("--kill-build-procs")
    if args.concise:
        parts.append("--concise")
    for raw_arg in getattr(args, "cmake_args", []) or []:
        parts.append(f"--cmake-args={raw_arg}")
    if getattr(args, "windows_icon_svg", None):
        parts.extend(["--windows-icon-svg", args.windows_icon_svg])
    if getattr(args, "rust_runtime_sync", None):
        parts.extend(["--rust-runtime-sync", args.rust_runtime_sync])
    if getattr(args, "runtime_platform", None):
        parts.extend(["--runtime-platform", args.runtime_platform])
    for extra_arg in getattr(args, "extra_args", []) or []:
        if extra_arg == "--":
            continue
        parts.append(extra_arg)
    return " ".join(parts)


def register(parser: argparse.ArgumentParser, defaults: ParserDefaults) -> None:
    parser.add_argument("--tidy", action="store_true")
    add_profile_arg(parser, defaults)
    add_build_dir_arg(parser)
    add_kill_build_procs_args(parser)
    add_concise_arg(parser)
    parser.add_argument(
        "--cmake-args",
        action="append",
        default=[],
        metavar="ARGS",
        help=(
            "Extra CMake configure args string applied before build. "
            "Can be repeated. Recommended: --cmake-args=-DENABLE_LTO=OFF"
        ),
    )
    parser.add_argument(
        "--windows-icon-svg",
        default=None,
        help=(
            "Override Windows CLI icon SVG source. "
            "Only used for tracer_windows_rust_cli release-profile builds."
        ),
    )
    parser.add_argument(
        "--rust-runtime-sync",
        choices=["relaxed", "strict"],
        default=None,
        help=(
            "Rust Windows CLI runtime sync mode. "
            "`relaxed` probes current/core fallback build dirs; `strict` only uses the current build dir."
        ),
    )
    parser.add_argument(
        "--runtime-platform",
        choices=["windows"],
        default=None,
        help=(
            "Apply platform-owned runtime build targets. "
            "Currently `windows` expands tracer_core runtime DLL targets and "
            "enables the explicit Windows runtime flow for tracer_windows_rust_cli."
        ),
    )
    parser.add_argument("extra_args", nargs=argparse.REMAINDER)


def run(args: argparse.Namespace, ctx: Context) -> int:
    if args.app == "tracer_windows_rust_cli" and args.runtime_platform != "windows":
        print(
            "Error: `build --app tracer_windows_rust_cli` now requires "
            "`--runtime-platform windows`.",
            file=sys.stderr,
        )
        return 2
    build_dir_error = reject_unsupported_build_dir_override(
        ctx=ctx,
        app_name=args.app,
        build_dir_name=args.build_dir,
        command_name="build",
    )
    if build_dir_error != 0:
        return build_dir_error
    kill_build_procs = bool(args.kill_build_procs and not args.no_kill_build_procs)
    cmd = BuildCommand(ctx)
    ret = cmd.build(
        app_name=args.app,
        tidy=args.tidy,
        extra_args=args.extra_args,
        cmake_args=parse_cmake_args(getattr(args, "cmake_args", [])),
        build_dir_name=args.build_dir,
        profile_name=args.profile,
        windows_icon_svg=getattr(args, "windows_icon_svg", None),
        rust_runtime_sync=(
            getattr(args, "rust_runtime_sync", None)
            if args.app != "tracer_windows_rust_cli"
            else (getattr(args, "rust_runtime_sync", None) or "strict")
        ),
        runtime_platform=getattr(args, "runtime_platform", None),
        kill_build_procs=kill_build_procs,
        concise=bool(args.concise),
    )
    if ret != 0:
        print_failure_report(
            command=_build_command_text(args),
            exit_code=int(ret),
            next_action=f"Fix errors and rerun: {_build_command_text(args)}",
            app_name=args.app,
            repo_root=ctx.repo_root,
            stage="build",
            build_log_path=cmd.resolve_output_log_path(
                app_name=args.app,
                tidy=args.tidy,
                build_dir_name=args.build_dir,
                profile_name=args.profile,
            ),
            fallback_key_error_hint="Build failed. See command output above.",
            include_result_json=False,
        )
    return ret


COMMAND = CommandSpec(name="build", register=register, run=run)
