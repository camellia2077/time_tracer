from pathlib import Path
from tempfile import TemporaryDirectory
from unittest import TestCase

from tools.toolchain.commands.tidy.task_auto_fix import (
    plan_redundant_cast_actions,
    suggest_const_name,
    suggest_task_refactors,
)
from tools.toolchain.commands.tidy.task_log import parse_task_log, resolve_task_log_path


class TestTidyTaskAutomation(TestCase):
    def test_suggest_const_name_supports_rule_templates(self):
        self.assertEqual(suggest_const_name("payload"), "kPayload")
        self.assertEqual(suggest_const_name("headers_it"), "kHeadersIt")
        self.assertEqual(suggest_const_name("tree_period_argument"), "kTreePeriodArgument")
        self.assertEqual(suggest_const_name("ok"), "kOk")

    def test_resolve_task_log_path_uses_smallest_pending_task(self):
        with TemporaryDirectory() as temp_dir:
            tasks_dir = Path(temp_dir)
            (tasks_dir / "batch_002").mkdir(parents=True)
            (tasks_dir / "batch_001").mkdir(parents=True)
            first = tasks_dir / "batch_001" / "task_003.log"
            second = tasks_dir / "batch_002" / "task_011.log"
            first.write_text("File: a.cpp\n", encoding="utf-8")
            second.write_text("File: b.cpp\n", encoding="utf-8")

            resolved = resolve_task_log_path(tasks_dir)
            self.assertEqual(resolved, first)

    def test_plan_redundant_cast_actions_extracts_replacement(self):
        with TemporaryDirectory() as temp_dir:
            temp_root = Path(temp_dir)
            source_file = temp_root / "example.cpp"
            source_file.write_text(
                "auto value = static_cast<unsigned long long>(limit);\n",
                encoding="utf-8",
            )
            task_log = temp_root / "task_001.log"
            task_log.write_text(
                "\n".join(
                    [
                        f"File: {source_file}",
                        "============================================================",
                        (
                            f"{source_file}:1:14: warning: redundant explicit casting to the same "
                            "type 'unsigned long long' as the sub-expression, remove this casting "
                            "[readability-redundant-casting]"
                        ),
                    ]
                ),
                encoding="utf-8",
            )

            parsed = parse_task_log(task_log)
            actions = plan_redundant_cast_actions(parsed)

            self.assertEqual(len(actions), 1)
            self.assertEqual(actions[0].replacement, "limit")
            self.assertEqual(actions[0].line, 1)

    def test_suggest_task_refactors_detects_decode_and_protocol_patterns(self):
        with TemporaryDirectory() as temp_dir:
            temp_root = Path(temp_dir)
            source_file = temp_root / "runtime_codec_demo.cpp"
            source_file.write_text(
                "\n".join(
                    [
                        "void Demo(const json& payload) {",
                        '  const auto ok = TryReadBoolField(payload, "ok");',
                        '  const auto error_code = TryReadStringField(payload, "error_code");',
                        '  const auto error_category = TryReadStringField(payload, "error_category");',
                        "  if (ok.HasError()) {}",
                        "  if (!ok.value.has_value()) {}",
                        "  if (error_code.HasError()) {}",
                        "  if (!error_code.value.has_value()) {}",
                        "  if (error_category.HasError()) {}",
                        "  if (!error_category.value.has_value()) {}",
                        '  if (value == 0) return "zero";',
                        '  if (value == 1) return "one";',
                        '  if (value == 2) return "two";',
                        "}",
                    ]
                ),
                encoding="utf-8",
            )
            task_log = temp_root / "task_002.log"
            task_log.write_text(
                "\n".join(
                    [
                        f"File: {source_file}",
                        "============================================================",
                    ]
                ),
                encoding="utf-8",
            )

            parsed = parse_task_log(task_log)
            suggestions = suggest_task_refactors(parsed)
            kinds = {item["kind"] for item in suggestions}

            self.assertIn("decode_helper", kinds)
            self.assertIn("protocol_constants", kinds)
