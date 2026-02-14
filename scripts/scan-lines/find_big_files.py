import argparse
import os

from line_scan_common import CPP_EXTENSIONS, group_files, print_grouped_report, scan_directory


DEFAULT_TARGET_PATH = (
    r"C:\Computer\my_github\github_cpp\time_tracer\time_tracer_cpp"
    r"\apps\time_tracer\src"
)
DEFAULT_THRESHOLD = 300


def main(target_path: str, threshold: int) -> None:
    if not os.path.exists(target_path):
        print(f"错误: 找不到路径 -> {target_path}")
        return

    scanned_files = scan_directory(target_path, CPP_EXTENSIONS)
    grouped_results = group_files(
        scanned_files,
        include_predicate=lambda line_count: line_count > threshold,
        sort_descending=True,
    )
    print_grouped_report(
        root_path=target_path,
        grouped_data=grouped_results,
        threshold=threshold,
        criteria_text=">",
        output_text="按一级子文件夹分组 -> 组内按行数降序 (大到小)",
        item_label="超标文件",
    )


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="查找行数超阈值的 C/C++ 文件。")
    parser.add_argument(
        "--path",
        default=DEFAULT_TARGET_PATH,
        help="扫描目录（默认: apps/time_tracer/src）",
    )
    parser.add_argument(
        "--threshold",
        type=int,
        default=DEFAULT_THRESHOLD,
        help="大文件阈值，严格大于该值会被输出（默认: 300）",
    )
    args = parser.parse_args()
    main(args.path, args.threshold)
