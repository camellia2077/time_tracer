#!/usr/bin/env python3
from __future__ import annotations

import argparse
import hashlib
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path


@dataclass
class FileDiff:
    relative_path: str
    left_sha256: str
    right_sha256: str
    same_bytes: bool
    detail: str


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Audit report output consistency between two directories."
    )
    parser.add_argument("--left-dir", required=True, help="Baseline output directory.")
    parser.add_argument("--right-dir", required=True, help="Compared output directory.")
    parser.add_argument(
        "--pattern",
        default="*.md",
        help="Glob pattern under both directories (default: *.md).",
    )
    parser.add_argument(
        "--output",
        default="report-consistency-audit.md",
        help="Output markdown report path.",
    )
    parser.add_argument(
        "--run-verify-core",
        action="store_true",
        help="Run `python tools/run.py build --app tracer_core --profile fast --concise` before audit.",
    )
    parser.add_argument(
        "--fail-on-diff",
        action="store_true",
        help="Return non-zero when any byte mismatch is found.",
    )
    parser.add_argument(
        "--normalize-ext",
        default=".md",
        help=(
            "Comma-separated extension whitelist for text normalization "
            "(BOM removal + CRLF/CR -> LF) before compare. "
            "Example: .md,.txt,.json ; default: .md"
        ),
    )
    return parser.parse_args()


def resolve_repo_root() -> Path:
    script_path = Path(__file__).resolve()
    for parent in script_path.parents:
        if (parent / "tools" / "run.py").is_file():
            return parent
    raise RuntimeError("Cannot resolve repository root from script path.")


def sha256_hex(content: bytes) -> str:
    return hashlib.sha256(content).hexdigest()


def normalize_text_bytes(content: bytes) -> bytes:
    # Compare text semantically across platforms: ignore UTF-8 BOM and
    # newline-style differences (CRLF/CR vs LF).
    if content.startswith(b"\xef\xbb\xbf"):
        content = content[3:]
    return content.replace(b"\r\n", b"\n").replace(b"\r", b"\n")


def parse_normalize_extensions(raw_value: str) -> set[str]:
    extensions: set[str] = set()
    for token in raw_value.split(","):
        item = token.strip().lower()
        if not item:
            continue
        if not item.startswith("."):
            item = f".{item}"
        extensions.add(item)
    return extensions


def first_diff_detail(left: bytes, right: bytes) -> str:
    max_len = min(len(left), len(right))
    for index in range(max_len):
        if left[index] == right[index]:
            continue
        line = 1
        column = 1
        for value in left[:index]:
            if value == 0x0A:
                line += 1
                column = 1
            else:
                column += 1
        return f"first mismatch at line {line}, column {column}, byte offset {index}"

    if len(left) != len(right):
        return f"content length differs (left={len(left)}, right={len(right)})"
    return "unknown mismatch"


def run_verify_core(repo_root: Path) -> int:
    cmd = [
        sys.executable,
        "tools/run.py",
        "build",
        "--app",
        "tracer_core",
        "--profile",
        "fast",
        "--concise",
    ]
    completed = subprocess.run(cmd, cwd=str(repo_root), check=False)
    return int(completed.returncode)


def collect_relative_paths(root: Path, pattern: str) -> set[str]:
    paths: set[str] = set()
    for path in root.rglob(pattern):
        if path.is_file():
            paths.add(str(path.relative_to(root)).replace("\\", "/"))
    return paths


def audit_dirs(
    left_dir: Path,
    right_dir: Path,
    pattern: str,
    normalize_extensions: set[str],
) -> tuple[list[FileDiff], list[str]]:
    left_paths = collect_relative_paths(left_dir, pattern)
    right_paths = collect_relative_paths(right_dir, pattern)

    missing: list[str] = []
    diffs: list[FileDiff] = []

    all_paths = sorted(left_paths | right_paths)
    for rel_path in all_paths:
        left_path = left_dir / rel_path
        right_path = right_dir / rel_path
        if not left_path.is_file() or not right_path.is_file():
            missing.append(rel_path)
            continue

        left_bytes = left_path.read_bytes()
        right_bytes = right_path.read_bytes()
        compare_left = left_bytes
        compare_right = right_bytes
        suffix = Path(rel_path).suffix.lower()
        normalized = suffix in normalize_extensions
        if normalized:
            compare_left = normalize_text_bytes(left_bytes)
            compare_right = normalize_text_bytes(right_bytes)
        same_bytes = compare_left == compare_right
        diffs.append(
            FileDiff(
                relative_path=rel_path,
                left_sha256=sha256_hex(compare_left),
                right_sha256=sha256_hex(compare_right),
                same_bytes=same_bytes,
                detail=(
                    f"identical ({suffix} normalized)"
                    if same_bytes and normalized
                    else ("identical" if same_bytes else first_diff_detail(compare_left, compare_right))
                ),
            )
        )

    return diffs, missing


def write_report(
    output_path: Path,
    left_dir: Path,
    right_dir: Path,
    pattern: str,
    diffs: list[FileDiff],
    missing: list[str],
) -> None:
    mismatches = [item for item in diffs if not item.same_bytes]
    lines: list[str] = []
    lines.append("# Report Consistency Audit")
    lines.append("")
    lines.append(f"- left_dir: `{left_dir}`")
    lines.append(f"- right_dir: `{right_dir}`")
    lines.append(f"- pattern: `{pattern}`")
    lines.append(f"- compared_files: `{len(diffs)}`")
    lines.append(f"- mismatches: `{len(mismatches)}`")
    lines.append(f"- missing_pairs: `{len(missing)}`")
    lines.append("")

    if missing:
        lines.append("## Missing Pairs")
        lines.append("")
        for rel_path in missing:
            lines.append(f"- `{rel_path}`")
        lines.append("")

    lines.append("## Byte Mismatches")
    lines.append("")
    if not mismatches:
        lines.append("- none")
    else:
        for item in mismatches:
            lines.append(f"- `{item.relative_path}`")
            lines.append(f"  - detail: {item.detail}")
            lines.append(f"  - left_sha256: `{item.left_sha256}`")
            lines.append(f"  - right_sha256: `{item.right_sha256}`")
    lines.append("")

    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    args = parse_args()
    repo_root = resolve_repo_root()
    left_dir = Path(args.left_dir)
    right_dir = Path(args.right_dir)
    output_path = Path(args.output)
    if not output_path.is_absolute():
        output_path = repo_root / output_path

    if args.run_verify_core:
        verify_exit_code = run_verify_core(repo_root)
        if verify_exit_code != 0:
            print(f"Error: verify failed with exit code {verify_exit_code}.")
            return verify_exit_code

    normalize_extensions = parse_normalize_extensions(args.normalize_ext)
    if normalize_extensions:
        ext_str = ", ".join(sorted(normalize_extensions))
        print(f"Normalization enabled for extensions: {ext_str}")
    else:
        print("Normalization disabled for all extensions.")

    diffs, missing = audit_dirs(left_dir, right_dir, args.pattern, normalize_extensions)
    write_report(output_path, left_dir, right_dir, args.pattern, diffs, missing)
    print(f"Audit report written to {output_path}")

    has_diff = any(not item.same_bytes for item in diffs)
    has_missing = bool(missing)
    mismatch_count = sum(1 for item in diffs if not item.same_bytes)
    print(
        "Audit summary: "
        f"compared={len(diffs)}, mismatches={mismatch_count}, missing_pairs={len(missing)}"
    )
    if missing:
        print("Missing pairs:")
        for rel_path in missing:
            print(f"  - {rel_path}")
    if mismatch_count:
        print("Byte mismatches:")
        for item in diffs:
            if not item.same_bytes:
                print(f"  - {item.relative_path}: {item.detail}")

    if args.fail_on_diff and (has_diff or has_missing):
        return 2
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
