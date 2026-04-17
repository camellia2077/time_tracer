from __future__ import annotations

import argparse
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[4]

if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.toolchain.commands.shared.android_failure_summary import (  # noqa: E402
    build_android_failure_summary,
    render_android_failure_summary,
)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Summarize Android CI failure logs into a concise text report."
    )
    parser.add_argument("--profile", required=True, help="Android verify profile name")
    parser.add_argument("--build-log", required=True, help="Path to apps/android/build/build.log")
    parser.add_argument(
        "--aggregated-log",
        required=True,
        help="Path to out/test/.../logs/output.log",
    )
    parser.add_argument(
        "--full-log",
        required=True,
        help="Path to out/test/.../logs/output_full.log",
    )
    parser.add_argument(
        "--result-json",
        required=True,
        help="Path to out/test/.../result.json",
    )
    parser.add_argument(
        "--output",
        required=True,
        help="Output path for the summary text file",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    summary = build_android_failure_summary(
        profile=args.profile,
        build_log_path=Path(args.build_log),
        aggregated_log_path=Path(args.aggregated_log),
        full_log_path=Path(args.full_log),
        result_json_path=Path(args.result_json),
    )
    rendered = render_android_failure_summary(summary)
    output_path = Path(args.output)
    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(rendered, encoding="utf-8")
    sys.stdout.write(rendered)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
