import os
from collections import defaultdict
from dataclasses import dataclass
from pathlib import Path
from typing import Callable, Dict, Iterable, List


CPP_EXTENSIONS = {".cpp", ".h", ".hpp", ".c", ".cc", ".cxx"}
ROOT_GROUP_NAME = "[根目录 Root]"


@dataclass
class FileInfo:
    full_path: str
    rel_path: str
    line_count: int


def count_file_lines(file_path: str) -> int:
    try:
        with open(file_path, "r", encoding="utf-8", errors="ignore") as file:
            return sum(1 for _ in file)
    except Exception:
        return 0


def scan_directory(root_path: str, extensions: Iterable[str]) -> List[FileInfo]:
    scanned_files: List[FileInfo] = []
    normalized_extensions = {ext.lower() for ext in extensions}

    for root, _, files in os.walk(root_path):
        for name in files:
            if Path(name).suffix.lower() not in normalized_extensions:
                continue

            abs_path = os.path.abspath(os.path.join(root, name))
            try:
                rel_path = os.path.relpath(abs_path, root_path)
            except ValueError:
                continue

            scanned_files.append(
                FileInfo(
                    full_path=abs_path,
                    rel_path=rel_path,
                    line_count=count_file_lines(abs_path),
                )
            )

    return scanned_files


def group_files(
    files: Iterable[FileInfo],
    include_predicate: Callable[[int], bool],
    sort_descending: bool,
) -> Dict[str, List[FileInfo]]:
    grouped: Dict[str, List[FileInfo]] = defaultdict(list)

    for file_info in files:
        if not include_predicate(file_info.line_count):
            continue

        parts = file_info.rel_path.split(os.sep)
        group_name = parts[0] if len(parts) > 1 else ROOT_GROUP_NAME
        grouped[group_name].append(file_info)

    for group_name, group_files_list in grouped.items():
        grouped[group_name] = sorted(
            group_files_list,
            key=lambda file_info: file_info.line_count,
            reverse=sort_descending,
        )

    return grouped


def print_grouped_report(
    root_path: str,
    grouped_data: Dict[str, List[FileInfo]],
    threshold: int,
    criteria_text: str,
    output_text: str,
    item_label: str,
) -> None:
    print(f"扫描目录: {root_path}")
    print(f"统计标准: 单个文件 {criteria_text} {threshold} 行")
    print(f"输出方式: {output_text}")

    if not grouped_data:
        print(f"\n未发现符合条件的{item_label}。")
        return

    for group_name in sorted(grouped_data.keys()):
        file_list = grouped_data[group_name]
        print("\n" + "=" * 120)
        print(f"📂 文件夹: {group_name} (包含 {len(file_list)} 个{item_label})")
        print("-" * 120)
        print(f"{'文件绝对路径':<100} | {'行数':<10}")
        print("-" * 120)
        for file_info in file_list:
            print(f"{file_info.full_path:<100} | {file_info.line_count:<10}")
