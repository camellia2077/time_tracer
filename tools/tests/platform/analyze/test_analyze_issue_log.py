import json
import sys
from pathlib import Path
from tempfile import TemporaryDirectory
from unittest import TestCase

REPO_ROOT = Path(__file__).resolve().parents[4]

if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.toolchain.commands.analyze.issue_log import (  # noqa: E402
    issue_artifact_paths,
    list_issue_paths,
    load_issue_record,
    next_issue_path,
    normalize_issue_id,
    resolve_issue_path,
)
from tools.toolchain.commands.analyze.issue_model import (  # noqa: E402
    AnalyzeIssue,
    AnalyzeIssueLocation,
    render_toon,
)


class TestAnalyzeIssueLog(TestCase):
    def test_normalize_issue_id(self):
        self.assertEqual(normalize_issue_id("1"), "001")
        self.assertEqual(normalize_issue_id("011"), "011")
        self.assertIsNone(normalize_issue_id(None))

    def test_resolve_issue_path_prefers_json_and_loads_issue(self):
        with TemporaryDirectory() as tmp:
            issues_dir = Path(tmp) / "issues"
            batch_dir = issues_dir / "batch_001"
            batch_dir.mkdir(parents=True, exist_ok=True)
            payload = {
                "version": 1,
                "issue_id": "001",
                "batch_id": "batch_001",
                "workspace": "build_analyze_core_family",
                "source_scope": "core_family",
                "source_file": r"C:\repo\a.cpp",
                "rule_id": "core.DivideZero",
                "category": "core",
                "severity": "warning",
                "message": "Division by zero",
                "primary_location": {
                    "file": r"C:\repo\a.cpp",
                    "line": 4,
                    "col": 7,
                    "end_line": 0,
                    "end_col": 0,
                    "message": "Division by zero",
                },
                "events": [],
                "fingerprint": "abc123",
                "raw_report": {"path": r"C:\repo\run.sarif", "run_index": 1, "result_index": 1},
            }
            json_path = batch_dir / "issue_001.json"
            json_path.write_text(json.dumps(payload, indent=2), encoding="utf-8")
            toon_path = batch_dir / "issue_001.toon"
            toon_path.write_text("issue:\n  id: 001\n", encoding="utf-8")

            resolved = resolve_issue_path(issues_dir, issue_id_value="1")
            loaded = load_issue_record(toon_path)

            self.assertEqual(resolved, json_path)
            self.assertEqual(loaded.issue_id, "001")
            self.assertEqual(loaded.rule_id, "core.DivideZero")
            self.assertEqual(issue_artifact_paths(toon_path), [json_path, toon_path])

    def test_list_next_and_explicit_toon_load(self):
        with TemporaryDirectory() as tmp:
            issues_dir = Path(tmp) / "issues"
            batch_dir = issues_dir / "batch_002"
            batch_dir.mkdir(parents=True, exist_ok=True)
            issue = AnalyzeIssue(
                version=1,
                issue_id="002",
                batch_id="batch_002",
                workspace="build_analyze_core_family",
                source_scope="core_family",
                source_file=r"C:\repo\b.cpp",
                rule_id="core.NullDereference",
                category="core",
                severity="error",
                message="Dereference of null pointer",
                primary_location=AnalyzeIssueLocation(
                    file=r"C:\repo\b.cpp",
                    line=9,
                    col=3,
                    end_line=0,
                    end_col=0,
                    message="Dereference of null pointer",
                ),
                events=(
                    AnalyzeIssueLocation(
                        file=r"C:\repo\b.cpp",
                        line=5,
                        col=10,
                        end_line=0,
                        end_col=0,
                        message="p initialized here",
                    ),
                ),
                fingerprint="f2",
                raw_report={},
            )
            (batch_dir / "issue_002.toon").write_text(render_toon(issue), encoding="utf-8")

            issue_paths = list_issue_paths(issues_dir)
            next_path = next_issue_path(issues_dir)
            resolved = resolve_issue_path(
                issues_dir,
                batch_id="2",
                issue_id_value="002",
            )
            loaded = load_issue_record(batch_dir / "issue_002.toon")

            self.assertEqual(len(issue_paths), 1)
            self.assertEqual(next_path, batch_dir / "issue_002.toon")
            self.assertEqual(resolved, batch_dir / "issue_002.toon")
            self.assertEqual(loaded.primary_location.line, 9)
            self.assertEqual(loaded.events[0].message, "p initialized here")
