#!/usr/bin/env python3
from __future__ import annotations

import argparse
import hashlib
import json
import re
import sys
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Any

try:
    from markdown_it import MarkdownIt
    from markdown_it.token import Token
except ImportError as error:  # pragma: no cover - exercised in runtime environments.
    MarkdownIt = None  # type: ignore[assignment]
    Token = Any  # type: ignore[misc,assignment]
    _IMPORT_ERROR = error
else:
    _IMPORT_ERROR = None


@dataclass
class MarkdownStructure:
    heading_levels: list[int]
    heading_texts: list[str]
    unordered_list_depths: list[int]
    ordered_list_count: int
    code_fence_blocks: int
    table_rows: int
    blank_lines: int
    semantic_token_count: int
    semantic_sha256: str


@dataclass
class RenderCheckResult:
    relative_path: str
    structure_equal: bool
    difference_detail: str
    left_structure: MarkdownStructure
    right_structure: MarkdownStructure


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Check markdown semantic render parity between two directories."
    )
    parser.add_argument("--left-dir", required=True, help="Baseline markdown directory.")
    parser.add_argument("--right-dir", required=True, help="Compared markdown directory.")
    parser.add_argument(
        "--pattern",
        default="*.md",
        help="Glob pattern under both directories (default: *.md).",
    )
    parser.add_argument(
        "--output",
        default="report-markdown-render-check.json",
        help="Output JSON report path.",
    )
    parser.add_argument(
        "--fail-on-structure-diff",
        action="store_true",
        help="Return non-zero when any semantic render structure differs.",
    )
    return parser.parse_args()


def resolve_repo_root() -> Path:
    script_path = Path(__file__).resolve()
    for parent in script_path.parents:
        if (parent / "tools" / "run.py").is_file():
            return parent
    raise RuntimeError("Cannot resolve repository root from script path.")


_WHITESPACE_RE = re.compile(r"\s+")


def normalize_text(text: str) -> str:
    return _WHITESPACE_RE.sub(" ", text).strip()


def normalize_attrs(token: Token) -> list[tuple[str, str]]:
    attrs = token.attrs
    if attrs is None:
        return []

    if isinstance(attrs, dict):
        items = attrs.items()
    else:
        items = attrs
    normalized = [(str(key), normalize_text(str(value))) for key, value in items]
    normalized.sort(key=lambda item: item[0])
    return normalized


def normalized_token_type(raw_type: str) -> str:
    if raw_type in {"softbreak", "hardbreak"}:
        return "linebreak"
    return raw_type


def normalize_token_content(token_type: str, content: str) -> str:
    if token_type in {"fence", "code_block"}:
        return content.replace("\r\n", "\n").replace("\r", "\n").rstrip("\n")
    return normalize_text(content)


def canonicalize_inline_children(children: list[Token] | None) -> list[str]:
    if not children:
        return []

    canonical: list[str] = []
    for child in children:
        child_type = normalized_token_type(child.type)
        child_content = normalize_token_content(child_type, child.content or "")
        if child_type == "text" and not child_content:
            continue
        canonical.append(f"{child_type}:{child_content}")
    return canonical


def canonicalize_token(token: Token) -> dict[str, Any]:
    token_type = normalized_token_type(token.type)
    return {
        "type": token_type,
        "tag": token.tag,
        "nesting": token.nesting,
        "attrs": normalize_attrs(token),
        "content": normalize_token_content(token_type, token.content or ""),
        "info": normalize_text(token.info or ""),
        "children": canonicalize_inline_children(token.children),
    }


def parse_markdown_structure(text: str, parser: MarkdownIt) -> tuple[MarkdownStructure, list[dict[str, Any]]]:
    tokens = parser.parse(text)
    semantic_tokens = [canonicalize_token(token) for token in tokens]
    semantic_payload = json.dumps(semantic_tokens, ensure_ascii=False, sort_keys=True)
    semantic_sha256 = hashlib.sha256(semantic_payload.encode("utf-8")).hexdigest()

    heading_levels: list[int] = []
    heading_texts: list[str] = []
    unordered_list_depths: list[int] = []
    ordered_list_count = 0
    code_fence_blocks = 0
    table_rows = 0
    blank_lines = sum(1 for line in text.splitlines() if not line.strip())
    list_stack: list[str] = []

    for index, token in enumerate(tokens):
        if token.type == "heading_open":
            if token.tag.startswith("h") and len(token.tag) == 2 and token.tag[1].isdigit():
                heading_levels.append(int(token.tag[1]))
            if index + 1 < len(tokens) and tokens[index + 1].type == "inline":
                heading_texts.append(normalize_text(tokens[index + 1].content or ""))
            continue

        if token.type == "bullet_list_open":
            list_stack.append("bullet")
            continue
        if token.type == "ordered_list_open":
            ordered_list_count += 1
            list_stack.append("ordered")
            continue
        if token.type in {"bullet_list_close", "ordered_list_close"}:
            if list_stack:
                list_stack.pop()
            continue
        if token.type == "list_item_open" and list_stack and list_stack[-1] == "bullet":
            unordered_list_depths.append(max(len(list_stack) - 1, 0))
            continue

        if token.type in {"fence", "code_block"}:
            code_fence_blocks += 1
            continue
        if token.type == "tr_open":
            table_rows += 1

    structure = MarkdownStructure(
        heading_levels=heading_levels,
        heading_texts=heading_texts,
        unordered_list_depths=unordered_list_depths,
        ordered_list_count=ordered_list_count,
        code_fence_blocks=code_fence_blocks,
        table_rows=table_rows,
        blank_lines=blank_lines,
        semantic_token_count=len(semantic_tokens),
        semantic_sha256=semantic_sha256,
    )
    return structure, semantic_tokens


def first_semantic_diff_detail(left: list[dict[str, Any]], right: list[dict[str, Any]]) -> str:
    max_len = min(len(left), len(right))
    for index in range(max_len):
        if left[index] == right[index]:
            continue
        left_type = str(left[index].get("type", "unknown"))
        right_type = str(right[index].get("type", "unknown"))
        return (
            f"semantic token mismatch at index {index}: "
            f"left_type={left_type}, right_type={right_type}"
        )
    if len(left) != len(right):
        return f"semantic token count differs (left={len(left)}, right={len(right)})"
    return "unknown semantic mismatch"


def collect_relative_paths(root: Path, pattern: str) -> set[str]:
    paths: set[str] = set()
    for path in root.rglob(pattern):
        if path.is_file():
            paths.add(str(path.relative_to(root)).replace("\\", "/"))
    return paths


def main() -> int:
    if _IMPORT_ERROR is not None:
        print(
            "Error: markdown-it-py is required for semantic markdown checks. "
            f"Import failed: {_IMPORT_ERROR}",
            file=sys.stderr,
        )
        return 2

    args = parse_args()
    repo_root = resolve_repo_root()
    left_dir = Path(args.left_dir)
    right_dir = Path(args.right_dir)
    output_path = Path(args.output)
    if not output_path.is_absolute():
        output_path = repo_root / output_path

    parser = MarkdownIt("default")

    left_paths = collect_relative_paths(left_dir, args.pattern)
    right_paths = collect_relative_paths(right_dir, args.pattern)
    all_paths = sorted(left_paths | right_paths)

    missing_pairs: list[str] = []
    results: list[RenderCheckResult] = []
    for rel_path in all_paths:
        left_path = left_dir / rel_path
        right_path = right_dir / rel_path
        if not left_path.is_file() or not right_path.is_file():
            missing_pairs.append(rel_path)
            continue

        left_structure, left_semantic = parse_markdown_structure(
            left_path.read_text(encoding="utf-8", errors="ignore"),
            parser,
        )
        right_structure, right_semantic = parse_markdown_structure(
            right_path.read_text(encoding="utf-8", errors="ignore"),
            parser,
        )
        structure_equal = left_semantic == right_semantic
        results.append(
            RenderCheckResult(
                relative_path=rel_path,
                structure_equal=structure_equal,
                difference_detail=(
                    "identical semantic token stream"
                    if structure_equal
                    else first_semantic_diff_detail(left_semantic, right_semantic)
                ),
                left_structure=left_structure,
                right_structure=right_structure,
            )
        )

    payload = {
        "left_dir": str(left_dir),
        "right_dir": str(right_dir),
        "pattern": args.pattern,
        "compared_files": len(results),
        "missing_pairs": missing_pairs,
        "results": [
            {
                "relative_path": item.relative_path,
                "structure_equal": item.structure_equal,
                "difference_detail": item.difference_detail,
                "left_structure": asdict(item.left_structure),
                "right_structure": asdict(item.right_structure),
            }
            for item in results
        ],
    }

    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(
        json.dumps(payload, indent=2, ensure_ascii=False),
        encoding="utf-8",
    )
    print(f"Render snapshot check written to {output_path}")

    has_diff = any(not item.structure_equal for item in results)
    if args.fail_on_structure_diff and (has_diff or bool(missing_pairs)):
        return 2
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
