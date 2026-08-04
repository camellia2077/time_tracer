import json
import sys
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parents[1]
SRC_ROOT = PROJECT_ROOT / "src"
if str(SRC_ROOT) not in sys.path:
    sys.path.insert(0, str(SRC_ROOT))

from loc_scanner.comparison import compare_profile_reports


def _write_report(path: Path, findings: list[dict]) -> None:
    path.write_text(
        json.dumps(
            {
                "report_type": "loc_profile_context",
                "priority_results": [{"priority": 0, "findings": findings}],
            }
        ),
        encoding="utf-8",
    )


def _finding(path: str, lines: int) -> dict:
    return {
        "category": "libs",
        "component": "tracer_transport",
        "language": "cpp",
        "path": path,
        "lines": lines,
    }


def test_compare_profile_reports_tracks_added_removed_and_changed(tmp_path: Path) -> None:
    baseline = tmp_path / "baseline.json"
    current = tmp_path / "current.json"
    _write_report(
        baseline,
        [_finding("old.cpp", 600), _finding("changed.cpp", 500)],
    )
    _write_report(
        current,
        [_finding("changed.cpp", 320), _finding("new.cpp", 410)],
    )

    delta = compare_profile_reports(baseline, current)

    assert delta["summary"] == {
        "baseline_matched_files": 2,
        "current_matched_files": 2,
        "added_files": 1,
        "removed_files": 1,
        "changed_files": 1,
        "net_matched_files": 0,
    }
    assert delta["added"][0]["path"] == "new.cpp"
    assert delta["removed"][0]["path"] == "old.cpp"
    assert delta["changed"][0]["delta_lines"] == -180


def test_compare_profile_reports_rejects_non_profile_report(tmp_path: Path) -> None:
    baseline = tmp_path / "baseline.json"
    current = tmp_path / "current.json"
    baseline.write_text(json.dumps({"report_type": "line_scan"}), encoding="utf-8")
    _write_report(current, [])

    try:
        compare_profile_reports(baseline, current)
    except ValueError as error:
        assert "Not a profile scan report" in str(error)
    else:
        raise AssertionError("expected non-profile report to be rejected")
