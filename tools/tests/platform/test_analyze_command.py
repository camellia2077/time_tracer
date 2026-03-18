import json
import sys
from pathlib import Path
from tempfile import TemporaryDirectory
from unittest import TestCase
from unittest.mock import patch

REPO_ROOT = Path(__file__).resolve().parents[3]

if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.toolchain.commands.analyze.command import (  # noqa: E402
    _is_excluded_analyze_file,
    _format_progress_file,
    AnalyzeCommand,
    build_analyzer_command,
    build_summary,
    merge_sarif_reports,
)
from tools.toolchain.commands.analyze.workspace import (  # noqa: E402
    DEFAULT_ANALYZE_BUILD_DIR_NAME,
    resolve_analyze_build_dir_name,
)
from tools.toolchain.core.context import Context  # noqa: E402


def _write_text(path: Path, content: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content, encoding="utf-8")


class TestAnalyzeCommand(TestCase):
    def test_execute_auto_splits_after_success(self):
        with TemporaryDirectory() as tmp:
            repo_root = Path(tmp)
            app_dir = repo_root / "apps" / "demo"
            source_file = app_dir / "src" / "demo.cpp"
            build_dir = repo_root / "out" / "build" / "demo" / "build_analyze"

            _write_text(
                repo_root / "tools" / "toolchain" / "config.toml",
                """
[apps.demo]
path = "apps/demo"
backend = "default"
""".strip(),
            )
            _write_text(source_file, "int main() { return 0; }\n")
            build_dir.mkdir(parents=True, exist_ok=True)
            _write_text(build_dir / "CMakeCache.txt", "")
            _write_text(
                build_dir / "compile_commands.json",
                json.dumps(
                    [
                        {
                            "directory": str(repo_root),
                            "file": str(source_file),
                            "arguments": [
                                "clang++",
                                "-std=gnu++23",
                                "-c",
                                str(source_file),
                            ],
                        }
                    ]
                ),
            )

            ctx = Context(repo_root)

            def _fake_run(command, cwd=None, env=None, stdout=None, stderr=None, text=None, encoding=None, errors=None, check=None):
                output_path = Path(command[command.index("-o") + 1])
                output_path.parent.mkdir(parents=True, exist_ok=True)
                output_path.write_text(
                    json.dumps(
                        {
                            "runs": [
                                {
                                    "results": [
                                        {
                                            "ruleId": "core.NullDereference",
                                            "level": "warning",
                                            "message": {"text": "demo issue"},
                                            "locations": [
                                                {
                                                    "physicalLocation": {
                                                        "artifactLocation": {
                                                            "uri": f"file:///{source_file.as_posix()}"
                                                        },
                                                        "region": {
                                                            "startLine": 1,
                                                            "startColumn": 1,
                                                        },
                                                    }
                                                }
                                            ],
                                        }
                                    ]
                                }
                            ]
                        }
                    ),
                    encoding="utf-8",
                )

                class _Completed:
                    returncode = 0
                    stdout = ""

                return _Completed()

            with patch(
                "tools.toolchain.commands.analyze.command.subprocess.run",
                side_effect=_fake_run,
            ):
                ret = AnalyzeCommand(ctx).execute(
                    "demo",
                    build_dir_name="build_analyze",
                )

            self.assertEqual(ret, 0)
            summary = json.loads(
                (repo_root / "out" / "analyze" / "demo" / "build_analyze" / "summary.json").read_text(
                    encoding="utf-8"
                )
            )
            self.assertEqual(summary["totals"]["results"], 1)
            self.assertEqual(summary["split"]["issue_count"], 1)
            issue_dir = repo_root / "out" / "analyze" / "demo" / "build_analyze" / "issues" / "batch_001"
            self.assertTrue((issue_dir / "issue_001.json").exists())
            self.assertTrue((issue_dir / "issue_001.toon").exists())

    def test_excluded_analyze_file_skips_deps_and_out_tree(self):
        repo_root = Path(r"C:\repo")

        self.assertTrue(
            _is_excluded_analyze_file(
                repo_root / "out" / "build" / "demo" / "_deps" / "sqlite-src" / "sqlite3.c",
                repo_root,
            )
        )
        self.assertTrue(
            _is_excluded_analyze_file(
                repo_root / "out" / "build" / "demo" / "generated.cpp",
                repo_root,
            )
        )
        self.assertFalse(
            _is_excluded_analyze_file(
                repo_root / "libs" / "tracer_core" / "src" / "demo.cpp",
                repo_root,
            )
        )

    def test_format_progress_file_prefers_repo_relative_path(self):
        repo_root = Path(r"C:\repo")
        file_path = repo_root / "libs" / "tracer_core" / "src" / "demo.cpp"

        display = _format_progress_file(repo_root, file_path)

        self.assertEqual(display, "libs/tracer_core/src/demo.cpp")

    def test_build_analyzer_command_strips_build_only_flags(self):
        entry = {
            "arguments": [
                r"C:\msys64\ucrt64\bin\clang++.exe",
                "-std=gnu++23",
                "-c",
                "-o",
                r"foo.obj",
                "-MD",
                "-MF",
                r"foo.d",
                r"C:\repo\foo.cpp",
            ]
        }

        command = build_analyzer_command(entry, Path(r"C:\tmp\foo.sarif"))

        self.assertIn("--analyze", command)
        self.assertIn("--analyzer-output", command)
        self.assertIn("sarif", command)
        self.assertNotIn("-c", command)
        self.assertNotIn(r"foo.obj", command)
        self.assertNotIn("-MD", command)
        self.assertNotIn(r"foo.d", command)
        self.assertEqual(command[-1], r"C:\repo\foo.cpp")

    def test_merge_sarif_reports_and_build_summary(self):
        with TemporaryDirectory() as tmp:
            root = Path(tmp)
            report_one = root / "one.sarif"
            report_two = root / "two.sarif"
            payload_one = {
                "runs": [
                    {
                        "results": [
                            {
                                "ruleId": "core.NullDereference",
                                "locations": [
                                    {
                                        "physicalLocation": {
                                            "artifactLocation": {"uri": "file:///C:/repo/a.cpp"}
                                        }
                                    }
                                ],
                            }
                        ]
                    }
                ]
            }
            payload_two = {
                "runs": [
                    {
                        "results": [
                            {
                                "ruleId": "core.NullDereference",
                                "locations": [
                                    {
                                        "physicalLocation": {
                                            "artifactLocation": {"uri": "file:///C:/repo/b.cpp"}
                                        }
                                    }
                                ],
                            },
                            {
                                "ruleId": "core.DivideZero",
                                "locations": [
                                    {
                                        "physicalLocation": {
                                            "artifactLocation": {"uri": "file:///C:/repo/b.cpp"}
                                        }
                                    }
                                ],
                            },
                        ]
                    }
                ]
            }
            report_one.write_text(json.dumps(payload_one), encoding="utf-8")
            report_two.write_text(json.dumps(payload_two), encoding="utf-8")

            merged = merge_sarif_reports([report_one, report_two])
            summary = build_summary(
                workspace_name="build_analyze_core_family",
                source_scope="core_family",
                build_dir=Path(r"C:\repo\out\build\tracer_core_shell\build_analyze_core_family"),
                raw_report_path=Path(r"C:\repo\out\analyze\tracer_core_shell\build_analyze_core_family\reports\run.sarif"),
                log_path=Path(r"C:\repo\out\analyze\tracer_core_shell\build_analyze_core_family\analyze.log"),
                matched_units=5,
                analyzed_units=4,
                failed_units=[{"file": r"C:\repo\c.cpp", "exit_code": 1}],
                merged_sarif=merged,
            )

            self.assertEqual(len(merged["runs"]), 2)
            self.assertEqual(summary["totals"]["results"], 3)
            self.assertEqual(summary["totals"]["files_with_findings"], 2)
            self.assertEqual(summary["top_rules"][0]["rule_id"], "core.NullDereference")
            self.assertEqual(summary["top_rules"][0]["count"], 2)

    def test_resolve_analyze_build_dir_name_uses_scope_suffix(self):
        ctx = Context(REPO_ROOT)
        self.assertEqual(
            resolve_analyze_build_dir_name(ctx, source_scope="core_family"),
            "build_analyze_core_family",
        )
        self.assertEqual(
            resolve_analyze_build_dir_name(ctx, source_scope=None),
            DEFAULT_ANALYZE_BUILD_DIR_NAME,
        )
