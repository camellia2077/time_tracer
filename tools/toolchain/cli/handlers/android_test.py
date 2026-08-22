import argparse

from ...commands.cmd_build.gradle import build_gradle
from ...commands.shared.result_reporting import print_failure_report
from ...core.context import Context
from ..model import CommandSpec, ParserDefaults


ANDROID_TEST_MODULES = (
    "app",
    "contract",
    "feature-data",
    "feature-insights",
    "feature-record",
    "feature-ui-common",
    "runtime",
)


def register(parser: argparse.ArgumentParser, _: ParserDefaults) -> None:
    parser.add_argument(
        "--module",
        required=True,
        choices=ANDROID_TEST_MODULES,
        help="Android Gradle module whose debug unit tests to run.",
    )
    parser.add_argument(
        "--tests",
        dest="test_patterns",
        action="append",
        default=[],
        metavar="PATTERN",
        help="JUnit class or method pattern to pass to Gradle; can be repeated.",
    )
    parser.add_argument(
        "--concise",
        action="store_true",
        help="Reduce toolchain output while running the selected Android unit tests.",
    )
    parser.add_argument(
        "extra_args",
        nargs=argparse.REMAINDER,
        help="Additional Gradle arguments forwarded after the test selection.",
    )


def _command_text(args: argparse.Namespace) -> str:
    parts = ["python tools/run.py android-test", "--module", args.module]
    for pattern in args.test_patterns:
        parts.extend(["--tests", pattern])
    if args.concise:
        parts.append("--concise")
    parts.extend(arg for arg in args.extra_args if arg != "--")
    return " ".join(parts)


def run(args: argparse.Namespace, ctx: Context) -> int:
    extra_args = [arg for arg in args.extra_args if arg != "--"]
    for pattern in args.test_patterns:
        extra_args.extend(["--tests", pattern])
    task = f":{args.module}:testDebugUnitTest"
    ret = build_gradle(
        ctx=ctx,
        app_name="tracer_android",
        tidy=False,
        extra_args=extra_args,
        cmake_args=[],
        build_dir_name=None,
        profile_name=None,
        gradle_tasks_override=[task],
        output_mode="quiet" if args.concise else "live",
    )
    if ret != 0:
        command = _command_text(args)
        print_failure_report(
            command=command,
            exit_code=int(ret),
            next_action=f"Fix errors and rerun: {command}",
            app_name="tracer_android",
            repo_root=ctx.repo_root,
            stage="android-test",
            fallback_key_error_hint="Android unit tests failed. See command output above.",
            include_result_json=False,
        )
    return ret


COMMAND = CommandSpec(
    name="android-test",
    register=register,
    run=run,
    app_mode="none",
    add_app_path=False,
    help="Run selected Android module debug unit tests.",
)
