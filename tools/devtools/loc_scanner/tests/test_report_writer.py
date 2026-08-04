import json
import sys
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parents[1]
SRC_ROOT = PROJECT_ROOT / "src"
if str(SRC_ROOT) not in sys.path:
    sys.path.insert(0, str(SRC_ROOT))

from loc_scanner.report_writer import (
    save_profile_baseline,
    write_profile_comparison_report,
)


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
        "category": "tools",
        "component": "loc_scanner",
        "language": "py",
        "path": path,
        "lines": lines,
    }


def test_write_profile_comparison_report_writes_json_and_markdown(
    tmp_path: Path,
) -> None:
    baseline = tmp_path / "baseline.json"
    current = tmp_path / "current.json"
    _write_report(baseline, [_finding("old.py", 500)])
    _write_report(current, [_finding("new.py", 300)])

    paths = write_profile_comparison_report(
        workspace_root=tmp_path,
        profile_name="loc_scanner",
        baseline_path=baseline,
        current_path=current,
    )

    markdown = Path(paths["markdown"]).read_text(encoding="utf-8")
    report = json.loads(Path(paths["json"]).read_text(encoding="utf-8"))
    assert "# LOC Scanner Context Delta" in markdown
    assert "old.py" in markdown
    assert "new.py" in markdown
    assert report["report_type"] == "loc_profile_context_delta"
    assert paths["summary"]["added_files"] == 1
    assert paths["summary"]["removed_files"] == 1


def test_save_profile_baseline_copies_current_report(tmp_path: Path) -> None:
    current = tmp_path / "current.json"
    baseline = tmp_path / "baselines" / "before.json"
    current.write_text('{"report_type":"loc_profile_context"}\n', encoding="utf-8")

    result = save_profile_baseline(current_path=current, baseline_path=baseline)

    assert result == str(baseline.resolve())
    assert baseline.read_bytes() == current.read_bytes()
