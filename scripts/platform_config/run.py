#!/usr/bin/env python3
from __future__ import annotations

import argparse
import sys
from pathlib import Path

if __package__ in (None, ""):
    PACKAGE_PARENT = Path(__file__).resolve().parent.parent
    if str(PACKAGE_PARENT) not in sys.path:
        sys.path.insert(0, str(PACKAGE_PARENT))
    from platform_config import sync
else:
    from . import sync


def default_repo_root() -> Path:
    return Path(__file__).resolve().parents[2]


def parse_args() -> argparse.Namespace:
    repo_root = default_repo_root()
    parser = argparse.ArgumentParser(
        description="Generate platform config roots from canonical source config."
    )
    parser.add_argument(
        "--target",
        choices=("windows", "android", "both"),
        default="both",
        help="Platform target to generate.",
    )
    parser.add_argument(
        "--source-root",
        type=Path,
        default=repo_root / "apps" / "tracer_core" / "config",
        help="Canonical source config root.",
    )
    parser.add_argument(
        "--windows-output-root",
        type=Path,
        default=repo_root / "apps" / "tracer_cli" / "windows" / "config",
        help="Output root for generated Windows config.",
    )
    parser.add_argument(
        "--android-output-root",
        type=Path,
        default=repo_root
        / "apps"
        / "tracer_android"
        / "runtime"
        / "src"
        / "main"
        / "assets"
        / "tracer_core"
        / "config",
        help="Output root for generated Android config.",
    )
    parser.add_argument(
        "--apply",
        action="store_true",
        help="Write files to output roots. Default is dry-run.",
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
    return sync.run_generation(
        target=args.target,
        source_root=args.source_root.resolve(),
        windows_output_root=args.windows_output_root.resolve(),
        android_output_root=args.android_output_root.resolve(),
        apply=args.apply,
        show_diff=args.show_diff,
        allow_overwrite_source=args.allow_overwrite_source,
    )


if __name__ == "__main__":
    raise SystemExit(main())
