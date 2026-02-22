import argparse

from ...commands.cmd_quality.artifact_size import ArtifactSizeCommand
from ...core.context import Context
from ..common import add_profile_arg, parse_cmake_args
from ..model import CommandSpec, ParserDefaults


def register(parser: argparse.ArgumentParser, defaults: ParserDefaults) -> None:
    add_profile_arg(parser, defaults)
    parser.add_argument(
        "--build-dir",
        default=None,
        help="Override build directory name (default: build_artifact_size).",
    )
    parser.add_argument(
        "--targets",
        nargs="+",
        required=True,
        help="One or more CMake build targets used for size statistics.",
    )
    parser.add_argument(
        "--artifact-glob",
        default="*.exe",
        help="Artifact glob under <app>/<build_dir>/bin (default: *.exe).",
    )
    parser.add_argument(
        "--exclude-substr",
        action="append",
        default=[],
        metavar="TEXT",
        help=(
            "Exclude artifacts whose filename contains TEXT (case-insensitive). Can be repeated."
        ),
    )
    parser.add_argument(
        "--result-json",
        default=None,
        help=(
            "Output JSON path. Relative path is resolved against repo root. "
            "Default: apps/<app>/<build_dir>/artifact_size.json"
        ),
    )
    parser.add_argument(
        "--cmake-args",
        action="append",
        default=[],
        metavar="ARGS",
        help=("Extra CMake configure args string applied before build. Can be repeated."),
    )


def run(args: argparse.Namespace, ctx: Context) -> int:
    cmd = ArtifactSizeCommand(ctx)
    return cmd.execute(
        app_name=args.app,
        targets=args.targets,
        build_dir_name=args.build_dir,
        profile_name=args.profile,
        result_json=args.result_json,
        artifact_glob=args.artifact_glob,
        exclude_substrings=args.exclude_substr,
        cmake_args=parse_cmake_args(getattr(args, "cmake_args", [])),
    )


COMMAND = CommandSpec(name="artifact-size", register=register, run=run)
