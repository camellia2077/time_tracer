from pathlib import Path

from .config import LanguageConfig


def print_line_scan_header(config: LanguageConfig, mode: str, threshold: int) -> None:
    mode_text = "large-file" if mode == "over" else "small-file"
    over_comparator = ">=" if config.over_inclusive else ">"
    comparator = over_comparator if mode == "over" else "<"
    print("=" * 100)
    print(
        f"{config.display_name} line-count scan report "
        f"({mode_text} mode: {comparator} {threshold} lines)"
    )
    print("=" * 100)
    print()


def print_line_path_result(
    path: Path,
    mode: str,
    threshold: int,
    matched: list[dict],
) -> None:
    project_name = path.name if path.name else str(path)
    print(f"[SCAN] Scanning project: [{project_name}]")
    print(f"  Path: {path}")
    if matched:
        print(f"  Found {len(matched)} matching files:")
        category_names = {
            "production": "PRODUCTION",
            "tests": "TESTS",
            "other": "OTHER",
        }
        for category in ("production", "tests", "other"):
            category_files = [
                file_result
                for file_result in matched
                if file_result["category"] == category
            ]
            if not category_files:
                continue
            print(f"  [{category_names[category]}] {len(category_files)} files:")
            for file_result in category_files:
                print(
                    f'    {file_result["lines"]:<6} lines | '
                    f'File "{file_result["path"]}"'
                )
    elif mode == "over":
        print(f"  [OK] No files at or above the {threshold}-line threshold.")
    else:
        print(f"  [OK] No files below the {threshold}-line threshold.")
    print()
    print("-" * 100)
    print()


def print_dir_scan_header(threshold: int, max_depth: int | None) -> None:
    print("=" * 100)
    print(f"Directory file-density scan report (more than {threshold} files)")
    if max_depth is None:
        print("Maximum depth: unlimited")
    else:
        print(f"Maximum depth: <= {max_depth}")
    print("=" * 100)
    print()


def print_dir_path_result(
    path: Path,
    threshold: int,
    matched: list[tuple[str, int]],
) -> None:
    project_name = path.name if path.name else str(path)
    print(f"[SCAN] Scanning project: [{project_name}]")
    print(f"  Path: {path}")
    if matched:
        print(f"  Found {len(matched)} directories over the file threshold:")
        for dir_path, file_count in matched:
            print(f'  {file_count:<6} files | Directory "{dir_path}"')
    else:
        print(f"  [OK] No directory contains more than {threshold} code files.")
    print()
    print("-" * 100)
    print()


def print_missing_path(path: Path) -> None:
    print(f"[SCAN] Scanning project: [{path.name}]")
    print(f"  Path: {path}")
    print("  [ERROR] Path does not exist; scan skipped.")
    print()
    print("-" * 100)
    print()
