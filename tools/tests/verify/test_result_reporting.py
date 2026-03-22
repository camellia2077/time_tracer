import sys
from pathlib import Path
from unittest import TestCase
from unittest.mock import patch

from tools.toolchain.commands.shared import result_reporting
from tools.toolchain.services.log_analysis import summarize_failure_text


class _EncodingSensitiveStream:
    encoding = "gbk"

    def __init__(self) -> None:
        self._parts: list[str] = []

    def write(self, text: str) -> int:
        text.encode(self.encoding)
        self._parts.append(text)
        return len(text)

    def flush(self) -> None:
        return None

    def getvalue(self) -> str:
        return "".join(self._parts)


class TestResultReporting(TestCase):
    def test_print_failure_report_replaces_non_encodable_key_error_lines(self) -> None:
        stream = _EncodingSensitiveStream()
        fake_paths = {
            "output_root": Path("out/test/fake"),
            "result_json": Path("out/test/fake/result.json"),
            "aggregated_log": Path("out/test/fake/logs/output.log"),
            "output_log": Path("out/test/fake/logs/output.log"),
        }

        with (
            patch.object(result_reporting, "resolve_result_paths", return_value=fake_paths),
            patch.object(
                result_reporting,
                "summarize_failure_log",
                return_value=type(
                    "Summary",
                    (),
                    {
                        "key_error_lines": ["module path \u0132 detail"],
                        "primary_error": "fatal error: demo",
                        "likely_fix": "Add missing include.",
                    },
                )(),
            ),
            patch.object(sys, "stdout", stream),
            patch.object(sys, "__stdout__", stream),
        ):
            result_reporting.print_failure_report(
                command="python tools/run.py verify --app tracer_core_shell",
                exit_code=1,
                next_action="rerun verify",
                app_name="tracer_core_shell",
                repo_root=Path.cwd(),
                stage="build",
                build_log_path=Path("out/build/fake/build.log"),
            )

        output = stream.getvalue()
        self.assertIn("=== FAILURE REPORT ===", output)
        self.assertIn("stage: build", output)
        self.assertIn("build_log: out\\build\\fake\\build.log", output)
        self.assertIn("aggregated_log: out\\test\\fake\\logs\\output.log", output)
        self.assertIn("primary_error: fatal error: demo", output)
        self.assertIn("likely_fix: Add missing include.", output)
        self.assertIn("module path ? detail", output)
        self.assertNotIn("\u0132", output)

    def test_summarize_failure_text_extracts_primary_error_and_likely_fix(self) -> None:
        summary = summarize_failure_text(
            "\n".join(
                [
                    "378 |     std::cerr << \"[FAIL] Failed to write tracer exchange package bytes.\\n\";",
                    "apps/demo.cpp:387:10: error: no member named 'cerr' in namespace 'std'",
                    "387 |     std::cerr << \"[FAIL] Encrypt error: \" << value",
                    "10 errors generated.",
                    "ninja: build stopped: subcommand failed.",
                ]
            )
        )

        self.assertEqual(
            summary.primary_error,
            "apps/demo.cpp:387:10: error: no member named 'cerr' in namespace 'std'",
        )
        self.assertEqual(
            summary.likely_fix,
            "Add `#include <iostream>` to the failing source file before using standard stream objects.",
        )
