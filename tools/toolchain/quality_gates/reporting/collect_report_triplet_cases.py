#!/usr/bin/env python3
from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from gate_cases_loader import TripletFileCase


DEFAULT_CASES_CONFIG = "test/suites/tracer_windows_rust_cli/tests/gate_cases.toml"


FORMAT_SPECS: dict[str, tuple[str, str]] = {
    "md": ("md", "md"),
    "tex": ("tex", "tex"),
    "typ": ("typ", "typ"),
}


def _load_gate_cases_config(config_path: Path):
    script_dir = Path(__file__).resolve().parent
    script_dir_str = str(script_dir)
    if script_dir_str not in sys.path:
        sys.path.insert(0, script_dir_str)
    from gate_cases_loader import load_gate_cases_config

    return load_gate_cases_config(config_path)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=("Collect fixed day/month/range report cases for md/tex/typ snapshot checks.")
    )
    parser.add_argument(
        "--format",
        required=True,
        choices=sorted(FORMAT_SPECS.keys()),
        help="Report format: md, tex, typ.",
    )
    parser.add_argument(
        "--export-root",
        required=True,
        help="Export root (for example out/test/.../reports/markdown).",
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
        help="Validate UTF-8, LF-only newline, and trailing LF for each case.",
    )
    parser.add_argument(
        "--cases-config",
        default=DEFAULT_CASES_CONFIG,
        help=(f"Gate case config TOML path. Default: {DEFAULT_CASES_CONFIG}"),
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


def collect_file_cases(
    export_root: Path,
    output_dir: Path,
    extension: str,
    strict: bool,
    file_cases: tuple[TripletFileCase, ...],
) -> None:
    for file_case in file_cases:
        source_path = export_root / file_case.source_relative_path_template.format(ext=extension)
        case_name = file_case.name_template.format(ext=extension)
        if not source_path.is_file():
            raise FileNotFoundError(f"missing source report: {source_path}")
        content = source_path.read_bytes()
        if strict:
            check_text_policy(case_name, content)
        (output_dir / case_name).write_bytes(content)


def collect_query_range_case(
    cli_bin: Path,
    db_path: Path,
    output_dir: Path,
    strict: bool,
    format_arg: str,
    extension: str,
    output_name_template: str,
    range_argument: str,
) -> None:
    case_name = output_name_template.format(ext=extension)
    cmd = [
        str(cli_bin),
        "report",
        "render",
        "range",
        range_argument,
        "--format",
        format_arg,
        "--db",
        str(db_path),
    ]
    completed = subprocess.run(
        cmd,
        check=False,
        capture_output=True,
    )
    if completed.returncode != 0:
        stderr_text = completed.stderr.decode("utf-8", errors="replace")
        raise RuntimeError(
            f"range report render failed while collecting `{case_name}`: "
            f"exit={completed.returncode}, stderr={stderr_text}"
        )

    # Normalize stdout for cross-platform line ending stability.
    content = normalize_crlf_to_lf(completed.stdout)
    if strict:
        check_text_policy(case_name, content)
    (output_dir / case_name).write_bytes(content)


def main() -> int:
    args = parse_args()
    format_arg, extension = FORMAT_SPECS[str(args.format)]
    export_root = Path(args.export_root)
    output_dir = Path(args.output_dir)
    cli_bin = Path(args.cli_bin)
    db_path = Path(args.db_path)
    config_path = Path(args.cases_config)
    if not config_path.is_absolute():
        config_path = (Path.cwd() / config_path).resolve()

    if not export_root.is_dir():
        print(f"Error: export root does not exist: {export_root}")
        return 2
    if not cli_bin.is_file():
        print(f"Error: cli executable does not exist: {cli_bin}")
        return 2
    if not db_path.is_file():
        print(f"Error: database file does not exist: {db_path}")
        return 2

    try:
        cases_config = _load_gate_cases_config(config_path)
    except Exception as error:
        print(f"Error: failed to load gate case config: {config_path} ({error})")
        return 2

    output_dir.mkdir(parents=True, exist_ok=True)
    strict = bool(args.strict_text_policy)
    collect_file_cases(
        export_root=export_root,
        output_dir=output_dir,
        extension=extension,
        strict=strict,
        file_cases=cases_config.triplet.file_cases,
    )
    collect_query_range_case(
        cli_bin=cli_bin,
        db_path=db_path,
        output_dir=output_dir,
        strict=strict,
        format_arg=format_arg,
        extension=extension,
        output_name_template=cases_config.triplet.range_case_name_template,
        range_argument=cases_config.triplet.range_argument,
    )
    print(f"Collected {args.format} triplet cases to {output_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
