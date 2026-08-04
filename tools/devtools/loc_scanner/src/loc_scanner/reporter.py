from pathlib import Path

from .config import LanguageConfig, ScanProfile
from .line_console_renderer import (
    print_dir_path_result,
    print_dir_scan_header,
    print_line_path_result,
    print_line_scan_header,
    print_missing_path,
)
from .profile_console_renderer import print_profile_scan_report


class LocConsoleReporter:
    """Compatibility facade for the scanner's console output protocols."""

    def __init__(self, config: LanguageConfig):
        self.config = config

    def print_line_scan_header(self, mode: str, threshold: int) -> None:
        print_line_scan_header(self.config, mode, threshold)

    def print_line_path_result(
        self,
        path: Path,
        mode: str,
        threshold: int,
        matched: list[dict],
    ) -> None:
        print_line_path_result(path, mode, threshold, matched)

    def print_dir_scan_header(self, threshold: int, max_depth: int | None) -> None:
        print_dir_scan_header(threshold, max_depth)

    def print_dir_path_result(
        self,
        path: Path,
        threshold: int,
        matched: list[tuple[str, int]],
    ) -> None:
        print_dir_path_result(path, threshold, matched)

    def print_profile_scan_report(
        self,
        *,
        profile: ScanProfile,
        mode: str,
        groups: list[dict],
        priority_groups: list[dict],
        missing_paths: list[Path],
        report_paths: dict[str, str],
        module_summaries: list[dict] | None = None,
        module_reading_candidates: list[dict] | None = None,
    ) -> None:
        print_profile_scan_report(
            profile=profile,
            mode=mode,
            groups=groups,
            priority_groups=priority_groups,
            missing_paths=missing_paths,
            report_paths=report_paths,
            module_summaries=module_summaries,
            module_reading_candidates=module_reading_candidates,
        )

    @staticmethod
    def print_missing_path(path: Path) -> None:
        print_missing_path(path)
