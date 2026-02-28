#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
from dataclasses import asdict, dataclass
from pathlib import Path


@dataclass
class MarkdownStructure:
    heading_levels: list[int]
    heading_texts: list[str]
    unordered_list_depths: list[int]
    ordered_list_count: int
    code_fence_blocks: int
    table_rows: int
    blank_lines: int


@dataclass
class RenderCheckResult:
    relative_path: str
    structure_equal: bool
    left_structure: MarkdownStructure
    right_structure: MarkdownStructure


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Check markdown render-oriented structure parity between two directories."
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
        default="temp/report-markdown-render-check.json",
        help="Output JSON report path.",
    )
    parser.add_argument(
        "--fail-on-structure-diff",
        action="store_true",
        help="Return non-zero when any render-oriented structure differs.",
    )
    return parser.parse_args()


def parse_markdown_structure(text: str) -> MarkdownStructure:
    heading_levels: list[int] = []
    heading_texts: list[str] = []
    unordered_list_depths: list[int] = []
    ordered_list_count = 0
    code_fence_blocks = 0
    table_rows = 0
    blank_lines = 0

    in_code_fence = False
    for raw_line in text.splitlines():
        line = raw_line.rstrip("\n")
        stripped = line.strip()
        if not stripped:
            blank_lines += 1
            continue

        if stripped.startswith("```"):
            if not in_code_fence:
                code_fence_blocks += 1
            in_code_fence = not in_code_fence
            continue
        if in_code_fence:
            continue

        if stripped.startswith("#"):
            level = 0
            for char in stripped:
                if char == "#":
                    level += 1
                else:
                    break
            if 1 <= level <= 6 and len(stripped) > level and stripped[level] == " ":
                heading_levels.append(level)
                heading_texts.append(stripped[level + 1 :].strip())
                continue

        leading_spaces = len(line) - len(line.lstrip(" "))
        if stripped.startswith("- ") or stripped.startswith("* "):
            unordered_list_depths.append(leading_spaces // 2)
            continue

        dot_pos = stripped.find(". ")
        if dot_pos > 0 and stripped[:dot_pos].isdigit():
            ordered_list_count += 1
            continue

        if "|" in stripped:
            table_rows += 1

    return MarkdownStructure(
        heading_levels=heading_levels,
        heading_texts=heading_texts,
        unordered_list_depths=unordered_list_depths,
        ordered_list_count=ordered_list_count,
        code_fence_blocks=code_fence_blocks,
        table_rows=table_rows,
        blank_lines=blank_lines,
    )


def collect_relative_paths(root: Path, pattern: str) -> set[str]:
    paths: set[str] = set()
    for path in root.rglob(pattern):
        if path.is_file():
            paths.add(str(path.relative_to(root)).replace("\\", "/"))
    return paths


def main() -> int:
    args = parse_args()
    repo_root = Path(__file__).resolve().parents[2]
    left_dir = Path(args.left_dir)
    right_dir = Path(args.right_dir)
    output_path = Path(args.output)
    if not output_path.is_absolute():
        output_path = repo_root / output_path

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

        left_structure = parse_markdown_structure(
            left_path.read_text(encoding="utf-8", errors="ignore")
        )
        right_structure = parse_markdown_structure(
            right_path.read_text(encoding="utf-8", errors="ignore")
        )
        results.append(
            RenderCheckResult(
                relative_path=rel_path,
                structure_equal=left_structure == right_structure,
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
