import sys
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parents[1]
SRC_ROOT = PROJECT_ROOT / "src"
if str(SRC_ROOT) not in sys.path:
    sys.path.insert(0, str(SRC_ROOT))

from loc_scanner.config import ComponentConfig, LanguageConfig, ScanProfile
from loc_scanner.reporter import LocConsoleReporter


def _language_config() -> LanguageConfig:
    return LanguageConfig(
        lang="py",
        display_name="Python",
        default_paths=[],
        extensions={".py"},
        ignore_dirs=set(),
        ignore_prefixes=(),
        path_mode="cli_override",
        default_over_threshold=200,
        default_under_threshold=120,
        default_dir_over_files=10,
        over_inclusive=True,
        test_directory_names=("test", "tests"),
    )


def test_line_reporter_uses_english_output(capsys) -> None:
    reporter = LocConsoleReporter(_language_config())

    reporter.print_line_scan_header("over", 200)
    reporter.print_line_path_result(
        Path("tools"),
        "over",
        200,
        [{"path": "tools/example.py", "lines": 250, "category": "production"}],
    )

    output = capsys.readouterr().out
    assert "line-count scan report" in output
    assert "Found 1 matching files" in output
    assert "lines" in output
    assert "扫描" not in output


def test_profile_reporter_uses_english_output(capsys) -> None:
    reporter = LocConsoleReporter(_language_config())
    profile = ScanProfile(
        name="loc_scanner",
        display_name="LOC Scanner",
        components=(
            ComponentConfig(
                name="loc_scanner",
                display_name="tools/devtools/loc_scanner",
                root="tools/devtools/loc_scanner",
                category="tools",
                priority=2,
            ),
        ),
        test_directory_names=("test", "tests"),
        test_priority=3,
    )

    reporter.print_profile_scan_report(
        profile=profile,
        mode="over",
        groups=[],
        priority_groups=[],
        missing_paths=[],
        report_paths={"markdown": "report.md", "json": "report.json"},
        module_summaries=[],
        module_reading_candidates=[],
    )

    output = capsys.readouterr().out
    assert "profile scan report" in output
    assert "No module reading candidates were generated." in output
    assert "Markdown report: report.md" in output
    assert "未生成" not in output
