from pathlib import Path
from tempfile import TemporaryDirectory
from unittest import TestCase

from tools.tests.platform.support.tidy_task_auto_fix_support import AutoFixFixtureBuilder, DiagnosticEntry
from tools.toolchain.commands.tidy.tasking.task_auto_fix import (
    plan_redundant_cast_actions,
    plan_runtime_int_actions,
    plan_using_namespace_actions,
    rename_candidates,
    suggest_const_name,
    suggest_task_refactors,
)


class TestTidyTaskAutoFixPlan(TestCase):
    def test_suggest_const_name_supports_rule_templates(self):
        self.assertEqual(suggest_const_name("payload"), "kPayload")
        self.assertEqual(suggest_const_name("headers_it"), "kHeadersIt")
        self.assertEqual(suggest_const_name("tree_period_argument"), "kTreePeriodArgument")
        self.assertEqual(suggest_const_name("ok"), "kOk")

    def test_plan_redundant_cast_actions_extracts_replacement(self):
        with TemporaryDirectory() as temp_dir:
            fb = AutoFixFixtureBuilder(Path(temp_dir))
            source_file = fb.write_source(
                "example.cpp",
                ["auto value = static_cast<unsigned long long>(limit);"],
            )
            task_log = fb.write_legacy_task_log(
                relative_path="task_001.log",
                source_file=source_file,
                diagnostics=[
                    DiagnosticEntry(
                        line=1,
                        col=14,
                        check="readability-redundant-casting",
                        message=(
                            "redundant explicit casting to the same type "
                            "'unsigned long long' as the sub-expression, remove this casting"
                        ),
                    )
                ],
            )

            parsed = fb.parse(task_log)
            actions = plan_redundant_cast_actions(parsed)

            self.assertEqual(len(actions), 1)
            self.assertEqual(actions[0].replacement, "limit")
            self.assertEqual(actions[0].line, 1)

    def test_plan_runtime_int_actions_extracts_replacement(self):
        with TemporaryDirectory() as temp_dir:
            fb = AutoFixFixtureBuilder(Path(temp_dir))
            source_file = fb.write_source(
                "example.cpp",
                [
                    '#include "example.hpp"',
                    "",
                    "long long duration = 0;",
                ],
            )
            task_log = fb.write_toon_task(
                relative_path="task_003.toon",
                task_id="003",
                batch_id="batch_001",
                source_file=source_file,
                diagnostics=[
                    DiagnosticEntry(
                        line=3,
                        col=1,
                        check="google-runtime-int",
                        message="consider replacing 'long long' with 'int64'",
                    )
                ],
            )

            parsed = fb.parse(task_log)
            actions = plan_runtime_int_actions(parsed)

            self.assertEqual(len(actions), 1)
            self.assertEqual(actions[0].kind, "runtime_int")
            self.assertEqual(actions[0].old_name, "long long")
            self.assertEqual(actions[0].new_name, "std::int64_t")
            self.assertEqual(actions[0].line, 3)

    def test_rename_candidates_fall_back_to_rule_template_for_toon_tasks(self):
        with TemporaryDirectory() as temp_dir:
            fb = AutoFixFixtureBuilder(Path(temp_dir))
            source_file = fb.write_source(
                "libs/tracer_adapters_io/src/infra/io/core/file_writer.cpp",
                [
                    "void FileWriter::WriteCanonicalText() {",
                    "  const std::string canonical = BuildCanonical();",
                    "  const auto bytes = ToUtf8Bytes(canonical);",
                    "}",
                ],
            )
            task_log = fb.write_toon_task(
                relative_path="task_029.toon",
                task_id="029",
                batch_id="batch_003",
                source_file=source_file,
                diagnostics=[
                    DiagnosticEntry(
                        line=2,
                        col=21,
                        check="readability-identifier-naming",
                        message="invalid case style for constant 'canonical'",
                    ),
                    DiagnosticEntry(
                        line=3,
                        col=14,
                        check="readability-identifier-naming",
                        message="invalid case style for constant 'bytes'",
                    ),
                ],
            )

            parsed = fb.parse(task_log)
            candidates = rename_candidates(parsed)

            self.assertEqual(len(candidates), 2)
            self.assertEqual(candidates[0]["new_name"], "kCanonical")
            self.assertEqual(candidates[1]["new_name"], "kBytes")

    def test_plan_using_namespace_actions_collects_symbols_from_dto_headers(self):
        with TemporaryDirectory() as temp_dir:
            fb = AutoFixFixtureBuilder(Path(temp_dir))
            fb.write_source(
                "libs/tracer_core/src/application/dto/reporting_responses.hpp",
                [
                    "#ifndef REPORTING_RESPONSES_HPP_",
                    "#define REPORTING_RESPONSES_HPP_",
                    "namespace tracer_core::core::dto {",
                    "enum class StructuredReportKind {",
                    "  kDay,",
                    "};",
                    "struct StructuredReportOutput {",
                    "  StructuredReportKind kind = StructuredReportKind::kDay;",
                    "};",
                    "}  // namespace tracer_core::core::dto",
                ],
            )
            fb.write_source(
                "libs/tracer_core/src/application/dto/shared_envelopes.hpp",
                [
                    "#ifndef SHARED_ENVELOPES_HPP_",
                    "#define SHARED_ENVELOPES_HPP_",
                    "namespace tracer_core::core::dto {",
                    "struct TextOutput {",
                    "  bool ok = true;",
                    "};",
                    "}  // namespace tracer_core::core::dto",
                ],
            )
            fb.write_source(
                "libs/tracer_core/src/application/use_cases/helper.hpp",
                [
                    '#include "application/dto/reporting_responses.hpp"',
                    '#include "application/dto/shared_envelopes.hpp"',
                    "namespace tracer::core::application::use_cases {",
                    "auto FormatStructured(",
                    "    const tracer_core::core::dto::StructuredReportOutput& output)",
                    "    -> tracer_core::core::dto::TextOutput;",
                    "}  // namespace tracer::core::application::use_cases",
                ],
            )
            source_file = fb.write_source(
                "libs/tracer_core/src/application/use_cases/helper.cpp",
                [
                    '#include "application/use_cases/helper.hpp"',
                    "",
                    "namespace tracer::core::application::use_cases {",
                    "",
                    "using namespace tracer_core::core::dto;",
                    "",
                    "auto FormatStructured(const StructuredReportOutput& output)",
                    "    -> TextOutput {",
                    "  return {.ok = output.kind == StructuredReportKind::kDay};",
                    "}",
                    "",
                    "}  // namespace tracer::core::application::use_cases",
                ],
            )
            task_log = fb.write_legacy_task_log(
                relative_path="task_006.log",
                source_file=source_file,
                diagnostics=[
                    DiagnosticEntry(
                        line=5,
                        col=1,
                        check="google-build-using-namespace",
                        message="do not use namespace using-directives; use using-declarations instead",
                    )
                ],
            )

            parsed = fb.parse(task_log)
            actions = plan_using_namespace_actions(parsed)

            self.assertEqual(len(actions), 1)
            self.assertEqual(
                actions[0].replacement.splitlines(),
                [
                    "using tracer_core::core::dto::StructuredReportOutput;",
                    "using tracer_core::core::dto::TextOutput;",
                    "using tracer_core::core::dto::StructuredReportKind;",
                ],
            )

    def test_plan_using_namespace_actions_supports_application_pipeline_chrono_namespace(self):
        with TemporaryDirectory() as temp_dir:
            fb = AutoFixFixtureBuilder(Path(temp_dir))
            source_file = fb.write_source(
                "libs/tracer_core/src/application/pipeline/pipeline_workflow.cpp",
                [
                    "#include <chrono>",
                    "",
                    "auto CurrentUnixMillis() -> std::int64_t {",
                    "  using namespace std::chrono;",
                    "  return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();",
                    "}",
                ],
            )
            task_log = fb.write_toon_task(
                relative_path="task_031.toon",
                task_id="031",
                batch_id="batch_004",
                source_file=source_file,
                diagnostics=[
                    DiagnosticEntry(
                        line=4,
                        col=3,
                        check="google-build-using-namespace",
                        message="do not use namespace using-directives; use using-declarations instead",
                    )
                ],
            )

            parsed = fb.parse(task_log)
            actions = plan_using_namespace_actions(parsed)

            self.assertEqual(len(actions), 1)
            self.assertEqual(actions[0].kind, "using_namespace")
            self.assertIn("using std::chrono::duration_cast;", actions[0].replacement)
            self.assertIn("using std::chrono::milliseconds;", actions[0].replacement)
            self.assertIn("using std::chrono::system_clock;", actions[0].replacement)

    def test_suggest_task_refactors_detects_decode_and_protocol_patterns(self):
        with TemporaryDirectory() as temp_dir:
            fb = AutoFixFixtureBuilder(Path(temp_dir))
            source_file = fb.write_source(
                "runtime_codec_demo.cpp",
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
                ],
            )
            task_log = fb.write_legacy_task_log(
                relative_path="task_002.log",
                source_file=source_file,
                diagnostics=[],
            )

            parsed = fb.parse(task_log)
            suggestions = suggest_task_refactors(parsed)
            kinds = {item["kind"] for item in suggestions}

            self.assertIn("decode_helper", kinds)
            self.assertIn("protocol_constants", kinds)
