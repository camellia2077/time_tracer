import argparse
from collections.abc import Sequence


def parse_suite_args(
    argv: Sequence[str],
    description: str,
    enable_format_on_success: bool,
):
    parser = argparse.ArgumentParser(description=description)
    parser.add_argument(
        "--build-dir",
        help="Build folder under project_apps_root, e.g. build_fast/build_tidy.",
    )
    parser.add_argument(
        "--bin-dir",
        help="Direct binary directory override (highest priority).",
    )
    parser.add_argument(
        "--config",
        help="Path to config.toml. Defaults to local config.toml.",
    )
    parser.add_argument(
        "--result-json",
        help=(
            "Result JSON path hint. Output contract fixes canonical path to "
            "output/<suite>/result.json; non-canonical values are ignored."
        ),
    )
    parser.add_argument(
        "--agent",
        action="store_true",
        help="Enable agent-friendly defaults.",
    )
    parser.add_argument(
        "--no-color",
        action="store_true",
        help="Disable ANSI color output.",
    )
    parser.add_argument(
        "--show-output",
        choices=["none", "fail", "all"],
        default="fail",
        help="Process output policy for tested executables.",
    )
    parser.add_argument(
        "--verbose",
        action="store_true",
        help="Show every test case output line.",
    )
    parser.add_argument(
        "--concise",
        action="store_true",
        help="Only show module summary and failure details.",
    )
    if enable_format_on_success:
        parser.add_argument(
            "--no-format-on-success",
            action="store_true",
            help="Disable post-test clang-format step.",
        )

    return parser.parse_args(argv)
