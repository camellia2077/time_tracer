#!/usr/bin/env python3
from __future__ import annotations

import argparse
import sys
from pathlib import Path

if __package__ in (None, ""):
    REPO_ROOT = Path(__file__).resolve().parents[2]
    if str(REPO_ROOT) not in sys.path:
        sys.path.insert(0, str(REPO_ROOT))
    from tools.platform_config import sync
    from tools.platform_paths import (
        CONFIG_PROFILES,
        android_config_root,
        tracer_core_config_root,
        windows_cli_config_root,
    )
else:
    from tools.platform_config import sync
    from tools.platform_paths import (
        CONFIG_PROFILES,
        android_config_root,
        tracer_core_config_root,
        windows_cli_config_root,
    )


def default_repo_root() -> Path:
    return Path(__file__).resolve().parents[2]


def parse_args() -> argparse.Namespace:
    repo_root = default_repo_root()
    parser = argparse.ArgumentParser(
        description="Generate platform config roots from canonical source config."
    )
    mode_group = parser.add_mutually_exclusive_group()
    parser.add_argument(
        "--target",
        choices=("windows", "android", "both"),
        default="both",
        help="Platform target to generate.",
    )
    parser.add_argument(
        "--config-profile",
        choices=CONFIG_PROFILES,
        default="test",
        help="Config source profile to generate (default: test).",
    )
    parser.add_argument(
        "--source-root",
        type=Path,
        default=None,
        help="Canonical source config root (overrides --config-profile).",
    )
    parser.add_argument(
        "--windows-output-root",
        type=Path,
        default=windows_cli_config_root(repo_root),
        help="Output root for generated Windows config.",
    )
    parser.add_argument(
        "--android-output-root",
        type=Path,
        default=android_config_root(repo_root),
        help="Output root for generated Android config.",
    )
    mode_group.add_argument(
        "--apply",
        action="store_true",
        help="Write files to output roots. Default is dry-run.",
    )
    mode_group.add_argument(
        "--check",
        action="store_true",
        help="Fail when synchronized output roots drift from canonical source config.",
    )
    parser.add_argument(
        "--show-diff",
        action="store_true",
        help="Print bundle.toml diff for each target.",
    )
    parser.add_argument(
        "--allow-overwrite-source",
        action="store_true",
        help="Allow output root to be equal to source root.",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    source_root = args.source_root or tracer_core_config_root(
        default_repo_root(), args.config_profile
    )
    return sync.run_generation(
        target=args.target,
        source_root=source_root.resolve(),
        windows_output_root=args.windows_output_root.resolve(),
        android_output_root=args.android_output_root.resolve(),
        apply=args.apply,
        check=args.check,
        show_diff=args.show_diff,
        allow_overwrite_source=args.allow_overwrite_source,
    )


if __name__ == "__main__":
    raise SystemExit(main())
