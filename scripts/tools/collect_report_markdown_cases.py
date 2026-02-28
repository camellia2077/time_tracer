#!/usr/bin/env python3
from __future__ import annotations

import argparse
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path


@dataclass(frozen=True)
class FileCase:
    name: str
    source_relative_path: str


FILE_CASES: tuple[FileCase, ...] = (
    FileCase(
        name="day_2026-01-03.md",
        source_relative_path="day/2026/01/2026-01-03.md",
    ),
    FileCase(
        name="month_2026-05.md",
        source_relative_path="month/2026-05.md",
    ),
    FileCase(
        name="week_2026-W05.md",
        source_relative_path="week/2026-W05.md",
    ),
    FileCase(
        name="year_2025.md",
        source_relative_path="year/2025.md",
    ),
    FileCase(
        name="recent_last_7_days.md",
        source_relative_path="recent/last_7_days_report.md",
    ),
)

RANGE_CASE_NAME = "range_2026-01-01_2026-01-31.md"
RANGE_ARGUMENT = "2026-01-01|2026-01-31"


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Collect fixed markdown report cases for golden/audit checks."
    )
    parser.add_argument(
        "--export-root",
        required=True,
        help="Export markdown root (for example test/output/.../reports/markdown).",
    )
    parser.add_argument(
        "--output-dir",
        required=True,
        help="Output directory containing normalized case files.",
    )
    parser.add_argument(
        "--cli-bin",
        required=True,
        help="time_tracer_cli executable path.",
    )
    parser.add_argument(
        "--db-path",
        required=True,
        help="SQLite database path used for range query capture.",
    )
    parser.add_argument(
        "--strict-text-policy",
        action="store_true",
        help="Validate UTF-8, LF-only newline, and trailing LF for every collected case.",
    )
    return parser.parse_args()


def check_text_policy(case_name: str, content: bytes) -> None:
    try:
        content.decode("utf-8")
    except UnicodeDecodeError as exc:
        raise RuntimeError(f"{case_name}: content is not UTF-8: {exc}") from exc

    if b"\r" in content:
        raise RuntimeError(f"{case_name}: contains CR byte; only LF is allowed.")
    if content and not content.endswith(b"\n"):
        raise RuntimeError(f"{case_name}: missing trailing LF newline.")


def normalize_crlf_to_lf(content: bytes) -> bytes:
    return content.replace(b"\r\n", b"\n").replace(b"\r", b"\n")


def collect_file_cases(export_root: Path, output_dir: Path, strict: bool) -> None:
    for file_case in FILE_CASES:
        source_path = export_root / file_case.source_relative_path
        if not source_path.is_file():
            raise FileNotFoundError(f"missing source report: {source_path}")
        content = source_path.read_bytes()
        if strict:
            check_text_policy(file_case.name, content)
        (output_dir / file_case.name).write_bytes(content)


def collect_range_case(
    cli_bin: Path,
    db_path: Path,
    output_dir: Path,
    strict: bool,
) -> None:
    cmd = [
        str(cli_bin),
        "query",
        "range",
        RANGE_ARGUMENT,
        "--format",
        "md",
        "--database",
        str(db_path),
    ]
    completed = subprocess.run(
        cmd,
        check=False,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    if completed.returncode != 0:
        stderr_text = completed.stderr.decode("utf-8", errors="replace")
        raise RuntimeError(
            "range query failed while collecting markdown case: "
            f"exit={completed.returncode}, stderr={stderr_text}"
        )
    # Range sample is captured from CLI stdout. On Windows, stdio may surface
    # CRLF despite core text policy being LF. Normalize here to keep sample
    # comparison transport-neutral.
    content = normalize_crlf_to_lf(completed.stdout)
    if strict:
        check_text_policy(RANGE_CASE_NAME, content)
    (output_dir / RANGE_CASE_NAME).write_bytes(content)


def main() -> int:
    args = parse_args()
    export_root = Path(args.export_root)
    output_dir = Path(args.output_dir)
    cli_bin = Path(args.cli_bin)
    db_path = Path(args.db_path)

    if not export_root.is_dir():
        print(f"Error: export root does not exist: {export_root}")
        return 2
    if not cli_bin.is_file():
        print(f"Error: cli executable does not exist: {cli_bin}")
        return 2
    if not db_path.is_file():
        print(f"Error: database file does not exist: {db_path}")
        return 2

    output_dir.mkdir(parents=True, exist_ok=True)
    collect_file_cases(
        export_root=export_root,
        output_dir=output_dir,
        strict=bool(args.strict_text_policy),
    )
    collect_range_case(
        cli_bin=cli_bin,
        db_path=db_path,
        output_dir=output_dir,
        strict=bool(args.strict_text_policy),
    )
    print(f"Collected markdown cases to {output_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
