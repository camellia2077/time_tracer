"""Run one clang-tidy invocation and persist its diagnostics as JSON.

This wrapper is intentionally small: CMake still owns the per-source
invocation, while the Python tidy pipeline owns task clustering and archival.
The build log remains available for humans, but it is not used as task input.
"""

from __future__ import annotations

import argparse
import json
import subprocess
import sys
from pathlib import Path


if __package__ in {None, ""}:
    repo_root = Path(__file__).resolve().parents[5]
    sys.path.insert(0, str(repo_root))

from tools.toolchain.commands.clang.tidy.diagnostics import extract_diagnostics
from tools.toolchain.commands.clang.tidy.models import ClangTidyInvocationResult


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--source-file", required=True, type=Path)
    parser.add_argument("command", nargs=argparse.REMAINDER)
    return parser


def run_wrapper(*, output: Path, source_file: Path, command: list[str]) -> int:
    if command and command[0] == "--":
        command = command[1:]
    if not command:
        raise ValueError("structured clang-tidy wrapper requires a command after --")

    completed = subprocess.run(
        command,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        encoding="utf-8",
        errors="replace",
        check=False,
    )
    combined = "\n".join(part for part in (completed.stdout, completed.stderr) if part)
    diagnostics = extract_diagnostics(combined.splitlines())
    for diagnostic in diagnostics:
        diagnostic.setdefault("source_file", str(source_file))

    payload = ClangTidyInvocationResult(
        source_file=source_file,
        exit_code=completed.returncode,
        diagnostics=tuple(diagnostics),
    ).to_dict()
    output.parent.mkdir(parents=True, exist_ok=True)
    temporary = output.with_suffix(output.suffix + ".tmp")
    temporary.write_text(json.dumps(payload, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    temporary.replace(output)

    if completed.stdout:
        sys.stdout.write(completed.stdout)
    if completed.stderr:
        sys.stderr.write(completed.stderr)
    return completed.returncode


def main(argv: list[str] | None = None) -> int:
    args = build_parser().parse_args(argv)
    return run_wrapper(
        output=args.output,
        source_file=args.source_file,
        command=list(args.command),
    )


if __name__ == "__main__":
    raise SystemExit(main())
