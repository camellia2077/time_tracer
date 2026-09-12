#!/usr/bin/env python3
from __future__ import annotations

import argparse
import hashlib
import json
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
        description="Audit insights output consistency between two directories."
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
        default="insights-consistency-audit.md",
        help="Output markdown insights path.",
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
            "(BOM/newline normalization; TeX/Typst lexical parsing) before compare. "
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


def _canonicalize_text_tokens(text: str) -> list[str]:
    tokens: list[str] = []
    current: list[str] = []
    for char in text:
        if char.isspace():
            if current:
                tokens.append("".join(current))
                current = []
            continue
        if char.isalnum() or char in "_-$":
            current.append(char)
            continue
        if current:
            tokens.append("".join(current))
            current = []
        tokens.append(char)
    if current:
        tokens.append("".join(current))
    return tokens


def canonicalize_latex_bytes(content: bytes) -> bytes:
    """Parse LaTeX into a stable node representation for golden comparison.

    The parser captures commands, environments, groups, math nodes, and text
    tokens while discarding source-only whitespace and comments.
    """
    try:
        from pylatexenc.latexwalker import (
            LatexCharsNode,
            LatexCommentNode,
            LatexEnvironmentNode,
            LatexGroupNode,
            LatexMacroNode,
            LatexMathNode,
            LatexWalker,
        )
    except ImportError as error:  # pragma: no cover - dependency is declared.
        raise RuntimeError(
            "pylatexenc is required for LaTeX golden checks; install project dependencies."
        ) from error

    def canonicalize_node(node):
        if isinstance(node, LatexCommentNode):
            return None
        if isinstance(node, LatexCharsNode):
            return ["text", *_canonicalize_text_tokens(node.chars)]
        if isinstance(node, LatexMacroNode):
            return [
                "macro",
                node.macroname,
                (
                    canonicalize_node(node.nodeoptarg)
                    if getattr(node, "nodeoptarg", None)
                    else None
                ),
                [
                    canonicalize_node(item)
                    for item in (getattr(node, "nodeargs", None) or [])
                ],
            ]
        if isinstance(node, LatexGroupNode):
            return [
                "group",
                node.delimiters,
                [canonicalize_node(item) for item in node.nodelist],
            ]
        if isinstance(node, LatexEnvironmentNode):
            return [
                "environment",
                node.environmentname,
                (
                    canonicalize_node(node.nodeoptarg)
                    if getattr(node, "nodeoptarg", None)
                    else None
                ),
                [
                    canonicalize_node(item)
                    for item in (getattr(node, "nodeargs", None) or [])
                ],
                [canonicalize_node(item) for item in node.nodelist],
            ]
        if isinstance(node, LatexMathNode):
            return [
                "math",
                node.math_mode,
                [canonicalize_node(item) for item in node.nodelist],
            ]
        return [type(node).__name__, getattr(node, "latex_verbatim", lambda: "")()]

    text = normalize_text_bytes(content).decode("utf-8")
    nodes, _, _ = LatexWalker(text).get_latex_nodes()
    canonical = [canonicalize_node(node) for node in nodes]
    return json.dumps(canonical, ensure_ascii=False, separators=(",", ":")).encode("utf-8")


def canonicalize_typst_bytes(content: bytes) -> bytes:
    """Compare Typst by lexical structure, not source formatting.

    Typst has no stable official Python AST package; retain the lightweight
    format-aware token representation until a maintained parser is available.
    """
    text = normalize_text_bytes(content).decode("utf-8")
    tokens: list[str] = []
    index = 0
    length = len(text)
    while index < length:
        char = text[index]
        if char.isspace():
            index += 1
            continue

        if text.startswith("//", index):
            newline = text.find("\n", index)
            index = length if newline < 0 else newline + 1
            continue
        if text.startswith("/*", index):
            end = text.find("*/", index + 2)
            index = length if end < 0 else end + 2
            continue

        if char in {'"', "'"}:
            quote = char
            end = index + 1
            escaped = False
            while end < length:
                current = text[end]
                if escaped:
                    escaped = False
                elif current == "\\":
                    escaped = True
                elif current == quote:
                    end += 1
                    break
                end += 1
            tokens.append(text[index:end])
            index = end
            continue

        if char.isalnum() or char in "_-$":
            end = index + 1
            while end < length and (text[end].isalnum() or text[end] in "_-$"):
                end += 1
            tokens.append(text[index:end])
            index = end
            continue

        tokens.append(char)
        index += 1

    return "\x1f".join(tokens).encode("utf-8")


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
            if suffix == ".tex":
                compare_left = canonicalize_latex_bytes(left_bytes)
                compare_right = canonicalize_latex_bytes(right_bytes)
            elif suffix == ".typ":
                compare_left = canonicalize_typst_bytes(left_bytes)
                compare_right = canonicalize_typst_bytes(right_bytes)
            else:
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
                    f"identical ({suffix} parsed tokens)"
                    if same_bytes and normalized
                    else ("identical" if same_bytes else first_diff_detail(compare_left, compare_right))
                ),
            )
        )

    return diffs, missing


def write_insights(
    output_path: Path,
    left_dir: Path,
    right_dir: Path,
    pattern: str,
    diffs: list[FileDiff],
    missing: list[str],
) -> None:
    mismatches = [item for item in diffs if not item.same_bytes]
    lines: list[str] = []
    lines.append("# Insights Consistency Audit")
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
    write_insights(output_path, left_dir, right_dir, args.pattern, diffs, missing)
    print(f"Audit insights written to {output_path}")

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
