#!/usr/bin/env python3
from __future__ import annotations

import argparse
import sys
from pathlib import Path

TOOLS_ROOT = Path(__file__).resolve().parent
REPO_ROOT = TOOLS_ROOT.parent
TEST_ROOT = REPO_ROOT / "test"
SUITES_ROOT = TOOLS_ROOT / "suites"
FRAMEWORK_ROOT = TOOLS_ROOT / "test_framework"


def _ensure_framework_path() -> None:
    repo_root_str = str(REPO_ROOT)
    if repo_root_str not in sys.path:
        sys.path.insert(0, repo_root_str)
    framework_root_str = str(FRAMEWORK_ROOT)
    if framework_root_str not in sys.path:
        sys.path.insert(0, framework_root_str)


def _load_config_func():
    _ensure_framework_path()
    from core.conf.loader import load_config

    return load_config


def _collect_suite_configs(suite_name: str | None) -> list[Path]:
    if suite_name:
        config_path = SUITES_ROOT / suite_name / "config.toml"
        return [config_path]
    return sorted(SUITES_ROOT.glob("*/config.toml"))


def main() -> int:
    load_config = _load_config_func()

    parser = argparse.ArgumentParser(
        description="Validate all test suite TOML schemas before execution."
    )
    parser.add_argument(
        "--suite",
        help="Only validate one suite folder (e.g. tracer_windows_rust_cli/log_generator).",
    )
    parser.add_argument(
        "--build-dir",
        default="build_fast",
        help="Build directory name passed to loader (default: build_fast).",
    )
    args = parser.parse_args()

    suite_configs = _collect_suite_configs(args.suite)
    if not suite_configs:
        print("No suite config.toml files found.")
        return 1

    failures = 0
    for config_path in suite_configs:
        try:
            load_config(config_path=config_path, build_dir_name=args.build_dir)
            print(f"PASS: {config_path.relative_to(REPO_ROOT)}")
        except Exception as error:
            failures += 1
            print(f"FAIL: {config_path.relative_to(REPO_ROOT)}")
            print(f"  -> {error}")

    if failures:
        print(f"Suite schema lint failed: {failures} suite(s).")
        return 1

    print("Suite schema lint passed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
