import argparse
import os
from typing import Iterable, List, Sequence

from line_scan_common import (
    CPP_EXTENSIONS,
    FileInfo,
    group_files,
    print_grouped_report,
    scan_directory,
)


DEFAULT_TARGET_PATH = (
    r"C:\Computer\my_github\github_cpp\time_tracer\time_tracer_cpp"
    r"\apps\time_tracer\src"
)
# 小文件行数阈值
DEFAULT_THRESHOLD = 40
DEFAULT_EXCLUDE_ROOT_FILES = True
DEFAULT_EXCLUDE_CONTRACT_FILES = True

CONTRACT_ROLE_SUFFIXES = (
    "_schema",
    "_manifest",
    "_registry",
    "_aliases",
)

DATA_CARRIER_SUFFIXES = (
    "_dto",
    "_request",
    "_requests",
    "_response",
    "_responses",
    "_model",
    "_models",
    "_types",
    "_config",
    "_data",
    "_requirements",
)

CONTRACT_PATH_MARKERS = (
    "/interfaces/",
    "/contracts/",
)

CONTRACT_BASENAMES = {
    "converter_config_models.hpp",
}


def _normalize_rel_path(path: str) -> str:
    return path.replace("\\", "/").strip("/")


def _is_excluded_directory(rel_dir: str, excluded_dirs: Sequence[str]) -> bool:
    if not rel_dir:
        return False

    for excluded_dir in excluded_dirs:
        normalized_excluded = _normalize_rel_path(excluded_dir)
        if not normalized_excluded:
            continue
        if rel_dir == normalized_excluded or rel_dir.startswith(
            normalized_excluded + "/"
        ):
            return True
    return False


def _is_contract_file(rel_path: str) -> bool:
    normalized_path = _normalize_rel_path(rel_path).lower()
    file_name = os.path.basename(normalized_path)
    stem, extension = os.path.splitext(file_name)

    # 接口契约: i_<capability>.hpp
    if stem.startswith("i_") and extension in (".h", ".hpp"):
        return True

    # schema/manifest/registry/aliases: 允许 .hpp/.cpp 均被识别
    if any(stem.endswith(suffix) for suffix in CONTRACT_ROLE_SUFFIXES):
        return True

    # 数据载体: 只在头文件维度排除（避免误伤实现 .cpp）
    if extension in (".h", ".hpp") and any(
        stem.endswith(suffix) for suffix in DATA_CARRIER_SUFFIXES
    ):
        return True

    if file_name in CONTRACT_BASENAMES:
        return True

    wrapped_path = f"/{normalized_path}/"
    for marker in CONTRACT_PATH_MARKERS:
        if marker in wrapped_path:
            return True

    return False


def _filter_files(
    scanned_files: Iterable[FileInfo],
    excluded_dirs: Sequence[str],
    exclude_root_files: bool,
    exclude_contract_files: bool,
) -> List[FileInfo]:
    result: List[FileInfo] = []

    for file_info in scanned_files:
        rel_dir = _normalize_rel_path(os.path.dirname(file_info.rel_path))

        if exclude_root_files and not rel_dir:
            continue

        if _is_excluded_directory(rel_dir, excluded_dirs):
            continue

        if exclude_contract_files and _is_contract_file(file_info.rel_path):
            continue

        result.append(file_info)

    return result


def main(
    target_path: str,
    threshold: int,
    excluded_dirs: Sequence[str],
    exclude_root_files: bool,
    exclude_contract_files: bool,
) -> None:
    if not os.path.exists(target_path):
        print(f"错误: 找不到路径 -> {target_path}")
        return

    scanned_files = scan_directory(target_path, CPP_EXTENSIONS)
    scanned_files = _filter_files(
        scanned_files,
        excluded_dirs,
        exclude_root_files,
        exclude_contract_files,
    )
    grouped_results = group_files(
        scanned_files,
        include_predicate=lambda line_count: line_count < threshold,
        sort_descending=False,
    )
    print_grouped_report(
        root_path=target_path,
        grouped_data=grouped_results,
        threshold=threshold,
        criteria_text="<",
        output_text="按一级子文件夹分组 -> 组内按行数升序 (小到大)",
        item_label="小文件",
    )


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="查找行数低于阈值的 C/C++ 文件。")
    parser.add_argument(
        "--path",
        default=DEFAULT_TARGET_PATH,
        help="扫描目录（默认: apps/time_tracer/src）",
    )
    parser.add_argument(
        "--threshold",
        type=int,
        default=DEFAULT_THRESHOLD,
        help="小文件阈值，严格小于该值会被输出（默认: 40）",
    )
    parser.add_argument(
        "--exclude-dir",
        action="append",
        default=[],
        help=(
            "屏蔽目录（相对 --path）。可重复传入，如 "
            "--exclude-dir application/tests --exclude-dir infrastructure/reports"
        ),
    )
    parser.add_argument(
        "--include-root-files",
        action="store_true",
        help="默认会跳过扫描根目录文件；加此参数后会重新纳入根目录文件。",
    )
    parser.add_argument(
        "--include-contract-files",
        action="store_true",
        help=(
            "默认会排除契约/数据载体命名文件（如 i_*, *_schema, *_dto, "
            "*_models, *_types, *_config, *_data, *_requirements）；加此参数后会重新纳入。"
        ),
    )
    args = parser.parse_args()
    main(
        args.path,
        args.threshold,
        excluded_dirs=args.exclude_dir,
        exclude_root_files=not args.include_root_files
        if DEFAULT_EXCLUDE_ROOT_FILES
        else False,
        exclude_contract_files=not args.include_contract_files
        if DEFAULT_EXCLUDE_CONTRACT_FILES
        else False,
    )
