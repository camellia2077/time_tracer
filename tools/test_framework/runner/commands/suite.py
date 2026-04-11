from __future__ import annotations

import argparse
import sys
from pathlib import Path

try:
    import tomllib
except ImportError:  # pragma: no cover
    import tomli as tomllib  # type: ignore

from runner.service import run_suite
from tools.toolchain.core.generated_paths import resolve_build_layout

from .run_support import (
    ensure_bin_dir_exists,
    load_suite_default_build_dir,
    resolve_build_dir,
    run_optional_build_steps,
)

SUITE_META = {
    # Artifact layer: core + windows cli integrated suite.
    "artifact_windows_cli": {
        "suite_folder": "tracer_windows_rust_cli",
        "suite_name": "artifact_windows_cli",
        "description": "[artifact] Core + Windows Rust CLI integrated executable suite (builds tracer_windows_rust_cli).",
        "format_app": None,
        "build_app": "tracer_windows_rust_cli",
    },
    "artifact_android": {
        "suite_folder": "tracer_android",
        "suite_name": "artifact_android",
        "description": "[artifact] Android host-side verification checks.",
        "format_app": None,
        "build_app": "tracer_android",
    },
    "artifact_log_generator": {
        "suite_folder": "log_generator",
        "suite_name": "artifact_log_generator",
        "description": "[artifact] Executable business-logic test runner for log_generator.",
        "format_app": None,
        "build_app": "log_generator",
    },
}

CANONICAL_SUITES = tuple(SUITE_META.keys())
CANONICAL_SUITE_TEXT = "|".join(CANONICAL_SUITES)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Unified test entry for all executable suites.")
    parser.add_argument(
        "--suite",
        default="artifact_windows_cli",
        choices=CANONICAL_SUITES,
        help=(f"Artifact suite to run. Canonical only: {CANONICAL_SUITE_TEXT}."),
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
            "Logical build dir name, resolved under out/build/<app>/<build_dir>. "
            "If omitted, uses suite TOML [paths].default_build_dir when set."
        ),
    )
    parser.add_argument(
        "--bin-dir",
        default=None,
        help="Direct executable directory override (highest priority for tests).",
    )
    return parser


def _resolve_effective_bin_dir(
    repo_root: Path,
    app_name: str,
    build_dir: str | None,
    requested_bin_dir: str | None,
) -> Path:
    if requested_bin_dir:
        return Path(requested_bin_dir).resolve()
    if not build_dir:
        raise RuntimeError(f"Cannot resolve bin dir for `{app_name}` without build dir.")
    return resolve_build_layout(repo_root, app_name, build_dir).bin_dir


def main(
    argv: list[str],
    suite_assets_root: Path,
    test_root: Path,
    repo_root: Path | None = None,
) -> int:
    parser = build_parser()
    args, forwarded = parser.parse_known_args(argv)
    suite_key = args.suite
    if suite_key not in SUITE_META:
        print(
            f"Error: unknown suite `{args.suite}`. Use one of: {CANONICAL_SUITE_TEXT}.",
            file=sys.stderr,
        )
        return 2
    meta = SUITE_META[suite_key]
    app_name = meta.get("build_app", suite_key)
    suite_folder = str(meta.get("suite_folder", meta["suite_name"]))
    suite_root = suite_assets_root / suite_folder
    active_repo_root = repo_root if repo_root else test_root.parent

    print(
        f"Suite `{suite_key}` (folder `{suite_folder}`) build target app: `{app_name}`",
        flush=True,
    )

    if not suite_root.exists():
        print(f"Suite folder not found: {suite_root}")
        return 1

    suite_default_build_dir = load_suite_default_build_dir(suite_root, tomllib.load)
    effective_build_dir = resolve_build_dir(
        repo_root=active_repo_root,
        app_name=str(app_name),
        requested_build_dir=args.build_dir,
        requested_bin_dir=args.bin_dir,
        suite_default_build_dir=suite_default_build_dir,
        with_build=args.with_build,
        skip_configure=args.skip_configure,
        skip_build=args.skip_build,
        tidy=args.tidy,
    )

    if args.with_build:
        tools_run = active_repo_root / "tools" / "run.py"
        if not tools_run.exists():
            print(f"Error: tools runner not found: {tools_run}")
            return 1

        build_exit_code = run_optional_build_steps(
            repo_root=active_repo_root,
            app_name=str(app_name),
            scripts_run=tools_run,
            python_exe=sys.executable,
            effective_build_dir=effective_build_dir,
            tidy=args.tidy,
            kill_build_procs=args.kill_build_procs,
            skip_configure=args.skip_configure,
            skip_build=args.skip_build,
        )
        if build_exit_code != 0:
            return int(build_exit_code)

    if not ensure_bin_dir_exists(
        repo_root=active_repo_root,
        app_name=str(app_name),
        build_dir=effective_build_dir,
        bin_dir=args.bin_dir,
    ):
        return 1

    resolved_bin_dir = _resolve_effective_bin_dir(
        repo_root=active_repo_root,
        app_name=str(app_name),
        build_dir=effective_build_dir,
        requested_bin_dir=args.bin_dir,
    )

    forwarded_args = list(forwarded)
    if not meta["format_app"]:
        forwarded_args = [
            item
            for item in forwarded_args
            if item not in {"--format-on-success", "--no-format-on-success"}
        ]
    forwarded_args.extend(["--bin-dir", str(resolved_bin_dir)])

    return run_suite(
        argv=forwarded_args,
        suite_root=suite_root,
        suite_name=meta["suite_name"],
        description=meta["description"],
        format_app=meta["format_app"],
        test_root=test_root,
    )
