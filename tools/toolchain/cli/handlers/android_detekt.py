import argparse

from ...commands.cmd_build import BuildCommand
from ...core.context import Context
from ..model import CommandSpec, ParserDefaults


ANDROID_DETEKT_PROFILE = "android_detekt"


def register(parser: argparse.ArgumentParser, _: ParserDefaults) -> None:
    parser.add_argument(
        "--concise",
        action="store_true",
        help="Reduce toolchain output while running all Android Detekt tasks.",
    )
    parser.add_argument(
        "extra_args",
        nargs=argparse.REMAINDER,
        help="Additional Gradle arguments forwarded after the Detekt task list.",
    )


def run(args: argparse.Namespace, ctx: Context) -> int:
    return BuildCommand(ctx).build(
        app_name="tracer_android",
        tidy=False,
        extra_args=list(args.extra_args or []),
        cmake_args=[],
        build_dir_name=None,
        profile_name=ANDROID_DETEKT_PROFILE,
        concise=bool(args.concise),
    )


COMMAND = CommandSpec(
    name="android-detekt",
    register=register,
    run=run,
    app_mode="none",
    add_app_path=False,
    help="Run Detekt for every Android module.",
)
