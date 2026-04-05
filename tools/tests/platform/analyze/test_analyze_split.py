import json
import sys
from pathlib import Path
from tempfile import TemporaryDirectory
from unittest import TestCase

REPO_ROOT = Path(__file__).resolve().parents[4]

if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.toolchain.commands.analyze.issue_model import (  # noqa: E402
    AnalyzeIssue,
    AnalyzeIssueLocation,
    render_toon,
)
from tools.toolchain.commands.analyze.sarif import uri_to_path  # noqa: E402
from tools.toolchain.commands.analyze.split import split_sarif_report  # noqa: E402


class TestAnalyzeSplit(TestCase):
    def test_uri_to_path_normalizes_windows_file_uri(self):
        self.assertEqual(
            uri_to_path("file:///C:/repo/foo.cpp"),
            r"C:\repo\foo.cpp",
        )

    def test_split_sarif_report_writes_issue_json_and_updates_summary(self):
        with TemporaryDirectory() as tmp:
            root = Path(tmp)
            raw_report_path = root / "run.sarif"
            issues_dir = root / "issues"
            summary_path = root / "summary.json"
            payload = {
                "runs": [
                    {
                        "artifacts": [
                            {"location": {"uri": "file:///C:/repo/a.cpp"}},
                            {"location": {"uri": "file:///C:/repo/b.cpp"}},
                        ],
                        "results": [
                            {
                                "ruleId": "core.NullDereference",
                                "level": "error",
                                "message": {"text": "Dereference of null pointer"},
                                "locations": [
                                    {
                                        "physicalLocation": {
                                            "artifactLocation": {"index": 1},
                                            "region": {"startLine": 9, "startColumn": 3},
                                        }
                                    }
                                ],
                                "codeFlows": [
                                    {
                                        "threadFlows": [
                                            {
                                                "locations": [
                                                    {
                                                        "location": {
                                                            "message": {"text": "p initialized here"},
                                                            "physicalLocation": {
                                                                "artifactLocation": {"index": 1},
                                                                "region": {
                                                                    "startLine": 5,
                                                                    "startColumn": 10,
                                                                },
                                                            },
                                                        }
                                                    }
                                                ]
                                            }
                                        ]
                                    }
                                ],
                            },
                            {
                                "ruleId": "core.DivideZero",
                                "level": "warning",
                                "message": {"text": "Division by zero"},
                                "locations": [
                                    {
                                        "physicalLocation": {
                                            "artifactLocation": {"index": 0},
                                            "region": {"startLine": 4, "startColumn": 7},
                                        }
                                    }
                                ],
                            },
                        ],
                    }
                ]
            }
            raw_report_path.write_text(json.dumps(payload), encoding="utf-8")
            summary_path.write_text(
                json.dumps({"workspace": "build_analyze_core_family"}, indent=2),
                encoding="utf-8",
            )

            stats = split_sarif_report(
                raw_report_path=raw_report_path,
                issues_dir=issues_dir,
                summary_path=summary_path,
                workspace_name="build_analyze_core_family",
                source_scope="core_family",
                batch_size=1,
            )

            self.assertEqual(stats["issues"], 2)
            self.assertEqual(stats["batches"], 2)

            issue_one = json.loads(
                (issues_dir / "batch_001" / "issue_001.json").read_text(encoding="utf-8")
            )
            issue_two = json.loads(
                (issues_dir / "batch_002" / "issue_002.json").read_text(encoding="utf-8")
            )
            issue_one_toon = (issues_dir / "batch_001" / "issue_001.toon").read_text(
                encoding="utf-8"
            )
            issue_two_toon = (issues_dir / "batch_002" / "issue_002.toon").read_text(
                encoding="utf-8"
            )
            summary = json.loads(summary_path.read_text(encoding="utf-8"))

            self.assertEqual(issue_one["source_file"], r"C:\repo\a.cpp")
            self.assertEqual(issue_one["rule_id"], "core.DivideZero")
            self.assertEqual(issue_one["batch_id"], "batch_001")
            self.assertEqual(issue_two["source_file"], r"C:\repo\b.cpp")
            self.assertEqual(issue_two["rule_id"], "core.NullDereference")
            self.assertEqual(issue_two["events"][0]["message"], "p initialized here")
            self.assertIn("issue:", issue_one_toon)
            self.assertIn("primary{file,line,col,end_line,end_col,message}:", issue_one_toon)
            self.assertIn("events[1]{index,file,line,col,end_line,end_col,message}:", issue_two_toon)
            self.assertIn("p initialized here", issue_two_toon)
            self.assertEqual(summary["split"]["issue_count"], 2)
            self.assertEqual(summary["split"]["batch_count"], 2)

    def test_render_toon_renders_primary_and_event_steps(self):
        issue = AnalyzeIssue(
            version=1,
            issue_id="001",
            batch_id="batch_001",
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
                end_line=9,
                end_col=4,
                message="Dereference of null pointer",
            ),
            events=(
                AnalyzeIssueLocation(
                    file=r"C:\repo\b.cpp",
                    line=5,
                    col=10,
                    end_line=5,
                    end_col=10,
                    message="p initialized here",
                ),
            ),
            fingerprint="abc123",
            raw_report={"path": r"C:\repo\run.sarif", "run_index": 1, "result_index": 2},
        )

        text = render_toon(issue)

        self.assertIn("issue:", text)
        self.assertIn("rule: core.NullDereference", text)
        self.assertIn(r"  C:\repo\b.cpp,9,3,9,4,Dereference of null pointer", text)
        self.assertIn(r"  1,C:\repo\b.cpp,5,10,5,10,p initialized here", text)
