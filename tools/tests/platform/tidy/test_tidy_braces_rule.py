from pathlib import Path
from tempfile import TemporaryDirectory
from types import SimpleNamespace
from unittest import TestCase

from tools.toolchain.commands.tidy.autofix.engines.text_edit_engine import TextEditEngine
from tools.toolchain.commands.tidy.autofix.models import FixContext
from tools.toolchain.commands.tidy.autofix.rules.braces_around_statements import (
    BracesAroundStatementsRule,
    build_braced_statement,
)


class TestTidyBracesRule(TestCase):
    def test_build_braced_statement_preserves_control_head_and_body(self):
        self.assertEqual(
            build_braced_statement(
                "  if (month < 10U) out << '0';",
                ("|                    {",),
            ),
            ("if (month < 10U) {", "  out << '0';", "}"),
        )

    def test_build_braced_statement_rejects_non_single_line_bodies(self):
        self.assertIsNone(
            build_braced_statement(
                "  if (ready) { Run(); }",
                ("|                    {",),
            )
        )
        self.assertIsNone(
            build_braced_statement(
                "  if (ready) Run(); // already handled",
                ("|                    {",),
            )
        )

    def test_rule_and_text_engine_apply_one_statement_braces(self):
        with TemporaryDirectory() as temp_dir:
            source_file = Path(temp_dir) / "example.cpp"
            source_file.write_text(
                "void Run(bool ready) {\n  if (ready) DoRun();\n}\n",
                encoding="utf-8",
            )
            diagnostic = SimpleNamespace(
                file=str(source_file),
                line=2,
                col=13,
                check="readability-braces-around-statements",
                raw_lines=("|                 {",),
            )
            context = FixContext(
                ctx=None,
                app_name="",
                workspace=None,
                parsed=SimpleNamespace(source_file=str(source_file)),
                build_tidy_dir=Path(temp_dir),
                dry_run=False,
            )
            intents = BracesAroundStatementsRule().plan(context, diagnostic)
            self.assertEqual(len(intents), 1)
            record = TextEditEngine().execute(context, intents)[0]

            self.assertEqual(record.status, "applied")
            self.assertEqual(record.reason, "braces_around_statement_added")
            self.assertEqual(
                source_file.read_text(encoding="utf-8"),
                "void Run(bool ready) {\n  if (ready) {\n    DoRun();\n  }\n}\n",
            )

    def test_rule_relocates_after_same_line_identifier_rename(self):
        with TemporaryDirectory() as temp_dir:
            source_file = Path(temp_dir) / "example.cpp"
            source_file.write_text(
                "void Run(bool ready) {\n  if (kReady) DoRun();\n}\n",
                encoding="utf-8",
            )
            context = FixContext(
                ctx=None,
                app_name="",
                workspace=None,
                parsed=SimpleNamespace(source_file=str(source_file)),
                build_tidy_dir=Path(temp_dir),
                dry_run=False,
            )
            diagnostic = SimpleNamespace(
                file=str(source_file),
                line=2,
                col=13,
                check="readability-braces-around-statements",
                raw_lines=("|                 {",),
            )
            intent = BracesAroundStatementsRule().plan(
                context,
                SimpleNamespace(
                    file=str(source_file),
                    line=2,
                    col=13,
                    check=diagnostic.check,
                    raw_lines=diagnostic.raw_lines,
                ),
            )[0]
            record = TextEditEngine().execute(context, [intent])[0]

            self.assertEqual(record.status, "applied")
            self.assertIn("if (kReady) {", source_file.read_text(encoding="utf-8"))
