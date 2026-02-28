import argparse
import sys
from pathlib import Path

try:
    import tomllib
except ImportError:  # pragma: no cover
    import tomli as tomllib  # type: ignore

test_root = Path(__file__).resolve().parent
framework_root = test_root / "framework"
sys.path.insert(0, str(framework_root))

from suite_runner import run_suite

from run_support import (
    _ensure_bin_dir_exists,
    _load_suite_default_build_dir,
    _resolve_build_dir,
    _run_optional_build_steps,
)

# `tracer_windows_cli` is the core + Windows CLI integrated suite.
SUITE_META = {
    "tracer_windows_cli": {
        "suite_name": "tracer_windows_cli",
        "description": "Core + Windows CLI integrated executable suite (builds tracer_windows_cli).",
        "format_app": "tracer_windows_cli",
        "build_app": "tracer_windows_cli",
    },
    "tracer_android": {
        "suite_name": "tracer_android",
        "description": "Android host-side verification checks.",
        "format_app": None,
        "build_app": "tracer_android",
    },
    "log_generator": {
        "suite_name": "log_generator",
        "description": "Executable business-logic test runner for log_generator.",
        "format_app": None,
        "build_app": "log_generator",
    },
}

ALIASES = {
    "tt": "tracer_windows_cli",
    "twc": "tracer_windows_cli",
    "ta": "tracer_android",
    "lg": "log_generator",
}


def parse_args(argv):
    parser = argparse.ArgumentParser(description="Unified test entry for all executable suites.")
    parser.add_argument(
        "--suite",
        default="tracer_windows_cli",
        choices=[
            "tracer_windows_cli",
            "tracer_android",
            "log_generator",
            "tt",
            "twc",
            "ta",
            "lg",
        ],
        help="Suite to run. Default: tracer_windows_cli.",
    )
    parser.add_argument(
        "--with-build",
        action="store_true",
        help="Run configure + build before tests.",
    )
    parser.add_argument(
        "--skip-configure",
        action="store_true",
        help="Skip configure stage when --with-build is enabled.",
    )
    parser.add_argument(
        "--skip-build",
        action="store_true",
        help="Skip build stage when --with-build is enabled.",
    )
    parser.add_argument(
        "--tidy",
        action="store_true",
        help="Build with clang-tidy enabled (default build dir becomes build_tidy).",
    )
    parser.add_argument(
        "--kill-build-procs",
        action="store_true",
        help="Pass kill-build-procs to configure/build stages.",
    )
    parser.add_argument(
        "--build-dir",
        default=None,
        help=(
            "Build folder under apps/<app>, e.g. build/build_fast/build_tidy. "
            "If omitted, uses suite TOML [paths].default_build_dir when set."
        ),
    )
    parser.add_argument(
        "--bin-dir",
        default=None,
        help="Direct executable directory override (highest priority for tests).",
    )
    return parser.parse_known_args(argv)


def main(argv=None):
    args, forwarded = parse_args(sys.argv[1:] if argv is None else argv)
    suite_key = ALIASES.get(args.suite, args.suite)
    meta = SUITE_META[suite_key]
    app_name = meta.get("build_app", suite_key)
    suite_root = test_root / "suites" / suite_key
    repo_root = test_root.parent

    print(
        f"Suite `{suite_key}` build target app: `{app_name}`",
        flush=True,
    )

    if not suite_root.exists():
        print(f"Suite folder not found: {suite_root}")
        return 1

    suite_default_build_dir = _load_suite_default_build_dir(suite_root, tomllib.load)
    effective_build_dir = _resolve_build_dir(
        repo_root=repo_root,
        app_name=app_name,
        requested_build_dir=args.build_dir,
        requested_bin_dir=args.bin_dir,
        suite_default_build_dir=suite_default_build_dir,
        with_build=args.with_build,
        skip_configure=args.skip_configure,
        skip_build=args.skip_build,
        tidy=args.tidy,
    )

    if args.with_build:
        scripts_run = repo_root / "scripts" / "run.py"
        if not scripts_run.exists():
            print(f"Error: scripts runner not found: {scripts_run}")
            return 1

        exit_code = _run_optional_build_steps(
            repo_root=repo_root,
            app_name=app_name,
            scripts_run=scripts_run,
            python_exe=sys.executable,
            effective_build_dir=effective_build_dir,
            tidy=args.tidy,
            kill_build_procs=args.kill_build_procs,
            skip_configure=args.skip_configure,
            skip_build=args.skip_build,
        )
        if exit_code != 0:
            return exit_code

    if not _ensure_bin_dir_exists(
        repo_root=repo_root,
        app_name=app_name,
        build_dir=effective_build_dir,
        bin_dir=args.bin_dir,
    ):
        return 1

    forwarded_args = list(forwarded)
    if args.bin_dir:
        forwarded_args.extend(["--bin-dir", args.bin_dir])
    elif effective_build_dir:
        forwarded_args.extend(["--build-dir", effective_build_dir])

    return run_suite(
        argv=forwarded_args,
        suite_root=suite_root,
        suite_name=meta["suite_name"],
        description=meta["description"],
        format_app=meta["format_app"],
        test_root=test_root,
    )


if __name__ == "__main__":
    sys.exit(main())
