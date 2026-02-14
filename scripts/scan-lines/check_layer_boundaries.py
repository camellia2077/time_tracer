import argparse
import os
import re
from dataclasses import dataclass
from typing import Dict, Iterable, List, Optional, Tuple
# 分层依赖越界检查

CPP_EXTENSIONS = {".cpp", ".cc", ".cxx", ".c", ".hpp", ".h"}
INCLUDE_PATTERN = re.compile(r'^\s*#\s*include\s*"([^"]+)"')
KNOWN_LAYERS = {"domain", "application", "api", "infrastructure", "shared"}


@dataclass
class Violation:
    file_path: str
    line_number: int
    source_layer: str
    include_path: str
    include_layer: str
    reason: str


def normalize(path: str) -> str:
    return path.replace("\\", "/")


def detect_layer_from_rel_path(rel_path: str) -> Optional[str]:
    normalized = normalize(rel_path)
    head = normalized.split("/", 1)[0]
    if head in KNOWN_LAYERS:
        return head
    return None


def detect_layer_from_include(include_path: str) -> Optional[str]:
    normalized = normalize(include_path)
    head = normalized.split("/", 1)[0]
    if head in KNOWN_LAYERS:
        return head
    return None


def iter_cpp_files(root_dir: str) -> Iterable[str]:
    for root, _, files in os.walk(root_dir):
        for name in files:
            ext = os.path.splitext(name)[1].lower()
            if ext in CPP_EXTENSIONS:
                yield os.path.join(root, name)


def check_include_rule(source_layer: str, include_layer: str) -> Optional[str]:
    if source_layer == "domain" and include_layer in {
        "application",
        "api",
        "infrastructure",
    }:
        return "domain cannot depend on application/api/infrastructure"

    if source_layer == "application" and include_layer in {"api", "infrastructure"}:
        return "application cannot depend on api/infrastructure implementations"

    if source_layer == "api" and include_layer == "infrastructure":
        return "api cannot depend on infrastructure implementations"

    return None


def scan_boundary_violations(root_dir: str) -> List[Violation]:
    violations: List[Violation] = []

    for file_path in iter_cpp_files(root_dir):
        rel_path = os.path.relpath(file_path, root_dir)
        source_layer = detect_layer_from_rel_path(rel_path)
        if source_layer is None:
            continue

        try:
            with open(file_path, "r", encoding="utf-8", errors="ignore") as handle:
                for line_no, line in enumerate(handle, start=1):
                    match = INCLUDE_PATTERN.match(line)
                    if not match:
                        continue

                    include_path = match.group(1)
                    include_layer = detect_layer_from_include(include_path)
                    if include_layer is None:
                        continue

                    reason = check_include_rule(source_layer, include_layer)
                    if reason is None:
                        continue

                    violations.append(
                        Violation(
                            file_path=normalize(file_path),
                            line_number=line_no,
                            source_layer=source_layer,
                            include_path=normalize(include_path),
                            include_layer=include_layer,
                            reason=reason,
                        )
                    )
        except OSError as exc:
            print(f"[warn] failed to read: {file_path} ({exc})")

    return violations


def summarize(violations: List[Violation]) -> Tuple[Dict[str, int], Dict[str, int]]:
    by_reason: Dict[str, int] = {}
    by_file: Dict[str, int] = {}
    for item in violations:
        by_reason[item.reason] = by_reason.get(item.reason, 0) + 1
        by_file[item.file_path] = by_file.get(item.file_path, 0) + 1
    return by_reason, by_file


def print_report(violations: List[Violation], max_show: int) -> None:
    print("Layer boundary check (time_tracer)")
    print("Rules:")
    print("- domain: no includes from application/api/infrastructure")
    print("- application: no includes from api/infrastructure")
    print("- api: no includes from infrastructure")
    print("- contract files (ports/interfaces/dto/abi) should remain independent")
    print()

    if not violations:
        print("No boundary violations found.")
        return

    by_reason, by_file = summarize(violations)
    print(f"Violations: {len(violations)}")
    print("By reason:")
    for reason, count in sorted(by_reason.items(), key=lambda kv: kv[1], reverse=True):
        print(f"- {reason}: {count}")

    print("Top files:")
    for file_path, count in sorted(by_file.items(), key=lambda kv: kv[1], reverse=True)[
        :10
    ]:
        print(f"- {file_path}: {count}")

    print()
    print(f"Details (showing up to {max_show}):")
    for item in violations[:max_show]:
        rel = item.file_path
        print(
            f"- {rel}:{item.line_number} -> include \"{item.include_path}\" "
            f"({item.source_layer} -> {item.include_layer})"
        )

    if len(violations) > max_show:
        print(f"... {len(violations) - max_show} more")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Check layer dependency boundaries by scanning #include paths."
    )
    parser.add_argument(
        "--root",
        default=r"apps/time_tracer/src",
        help="Root source directory to scan. Default: apps/time_tracer/src",
    )
    parser.add_argument(
        "--strict",
        action="store_true",
        help="Return non-zero exit code when violations are found.",
    )
    parser.add_argument(
        "--max-show",
        type=int,
        default=120,
        help="Maximum number of detailed violations to print.",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    root_dir = os.path.abspath(args.root)
    if not os.path.isdir(root_dir):
        print(f"Invalid root path: {root_dir}")
        return 2

    violations = scan_boundary_violations(root_dir)
    print_report(violations, max_show=max(0, args.max_show))

    if args.strict and violations:
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
