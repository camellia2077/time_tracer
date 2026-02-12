#!/usr/bin/env python3
from __future__ import annotations

import argparse
import sys
from pathlib import Path


SUITES_ROOT = Path(__file__).resolve().parent
TEST_ROOT = SUITES_ROOT.parent
FRAMEWORK_ROOT = TEST_ROOT / "framework"
sys.path.insert(0, str(FRAMEWORK_ROOT))

from core.conf.loader import load_config


def _collect_suite_configs(suite_name: str | None) -> list[Path]:
    if suite_name:
        config_path = SUITES_ROOT / suite_name / "config.toml"
        return [config_path]
    return sorted(SUITES_ROOT.glob("*/config.toml"))


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Validate all test suite TOML schemas before execution."
    )
    parser.add_argument(
        "--suite",
        help="Only validate one suite folder (e.g. time_tracer/log_generator).",
    )
    parser.add_argument(
        "--build-dir",
        default="build_agent",
        help="Build directory name passed to loader (default: build_agent).",
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
            print(f"PASS: {config_path.relative_to(TEST_ROOT)}")
        except Exception as error:
            failures += 1
            print(f"FAIL: {config_path.relative_to(TEST_ROOT)}")
            print(f"  -> {error}")

    if failures:
        print(f"Suite schema lint failed: {failures} suite(s).")
        return 1

    print("Suite schema lint passed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
