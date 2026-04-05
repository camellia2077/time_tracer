from tempfile import TemporaryDirectory
from unittest import TestCase

from tools.tests.platform.support.tidy_task_auto_fix_support import AutoFixFixtureBuilder, DiagnosticEntry
from tools.toolchain.commands.tidy.tasking.task_auto_fix import (
    apply_explicit_constructor_actions,
    apply_runtime_int_actions,
    apply_using_namespace_actions,
    plan_explicit_constructor_actions,
    plan_runtime_int_actions,
    plan_using_namespace_actions,
)


class TestTidyTaskAutoFixApply(TestCase):
    def test_apply_runtime_int_actions_rewrites_type_and_adds_cstdint_include(self):
        with TemporaryDirectory() as temp_dir:
            fb = AutoFixFixtureBuilder(temp_dir)
            source_file = fb.write_source(
                "example.cpp",
                [
                    "// sample file",
                    '#include "example.hpp"',
                    "",
                    "long long duration = 0;",
                ],
            )
            task_log = fb.write_legacy_task_log(
                relative_path="task_004.log",
                source_file=source_file,
                diagnostics=[
                    DiagnosticEntry(
                        line=4,
                        col=1,
                        check="google-runtime-int",
                        message="consider replacing 'long long' with 'int64'",
                    )
                ],
            )

            parsed = fb.parse(task_log)
            actions = plan_runtime_int_actions(parsed)
            self.assertEqual(len(actions), 1)

            apply_runtime_int_actions(actions, dry_run=False)

            updated = source_file.read_text(encoding="utf-8")
            self.assertIn("#include <cstdint>", updated)
            self.assertIn("std::int64_t duration = 0;", updated)
            self.assertEqual(actions[0].status, "applied")
            self.assertEqual(actions[0].reason, "google_runtime_int_replaced")

    def test_apply_explicit_constructor_actions_inserts_explicit_keyword(self):
        with TemporaryDirectory() as temp_dir:
            fb = AutoFixFixtureBuilder(temp_dir)
            source_file = fb.write_source(
                "text_validator.cpp",
                [
                    "struct PImpl {",
                    "  PImpl(const ConverterConfig& config) : line_processor(config) {}",
                    "};",
                ],
            )
            task_log = fb.write_toon_task(
                relative_path="task_018.toon",
                task_id="018",
                batch_id="batch_002",
                source_file=source_file,
                diagnostics=[
                    DiagnosticEntry(
                        line=2,
                        col=3,
                        check="google-explicit-constructor",
                        message=(
                            "single-argument constructors must be marked explicit to avoid "
                            "unintentional implicit conversions"
                        ),
                    )
                ],
            )

            parsed = fb.parse(task_log)
            actions = plan_explicit_constructor_actions(parsed)

            self.assertEqual(len(actions), 1)
            self.assertEqual(actions[0].kind, "explicit_constructor")

            apply_explicit_constructor_actions(actions, dry_run=False)

            updated = source_file.read_text(encoding="utf-8")
            self.assertIn("explicit PImpl(const ConverterConfig& config)", updated)
            self.assertEqual(actions[0].status, "applied")
            self.assertEqual(actions[0].reason, "google_explicit_constructor_added")

    def test_plan_using_namespace_actions_collects_symbols_from_companion_header(self):
        with TemporaryDirectory() as temp_dir:
            fb = AutoFixFixtureBuilder(temp_dir)
            fb.write_source(
                "libs/tracer_core/src/application/use_cases/pipeline_api.hpp",
                [
                    "#ifndef PIPELINE_API_HPP_",
                    "#define PIPELINE_API_HPP_",
                    "namespace tracer::core::application::use_cases {",
                    "class PipelineApi final {",
                    " public:",
                    "  auto RunConvert(const tracer_core::core::dto::ConvertRequest& request)",
                    "      -> tracer_core::core::dto::OperationAck;",
                    "};",
                    "}  // namespace tracer::core::application::use_cases",
                ],
            )
            source_file = fb.write_source(
                "libs/tracer_core/src/application/use_cases/pipeline_api.cpp",
                [
                    '#include "application/use_cases/pipeline_api.hpp"',
                    "",
                    "namespace tracer::core::application::use_cases {",
                    "",
                    "using namespace tracer_core::core::dto;",
                    "",
                    "auto PipelineApi::RunConvert(const ConvertRequest& request)",
                    "    -> OperationAck {",
                    "  return {.ok = true, .error_message = request.input_path};",
                    "}",
                    "",
                    "}  // namespace tracer::core::application::use_cases",
                ],
            )
            task_log = fb.write_toon_task(
                relative_path="libs/tracer_core/src/task_005.toon",
                task_id="005",
                batch_id="batch_001",
                source_file=source_file,
                diagnostics=[
                    DiagnosticEntry(
                        line=5,
                        col=1,
                        check="google-build-using-namespace",
                        message=(
                            "do not use namespace using-directives; use using-declarations "
                            "instead"
                        ),
                    )
                ],
            )

            parsed = fb.parse(task_log)
            actions = plan_using_namespace_actions(parsed)

            self.assertEqual(len(actions), 1)
            self.assertEqual(
                actions[0].replacement.splitlines(),
                [
                    "using tracer_core::core::dto::ConvertRequest;",
                    "using tracer_core::core::dto::OperationAck;",
                ],
            )

            before_text = source_file.read_text(encoding="utf-8")
            apply_using_namespace_actions(actions, dry_run=True)
            after_text = source_file.read_text(encoding="utf-8")

            self.assertEqual(before_text, after_text)
            self.assertEqual(actions[0].status, "previewed")
            self.assertEqual(actions[0].reason, "dto_using_declarations_preview")
            self.assertIn(
                "using tracer_core::core::dto::ConvertRequest;",
                actions[0].diff,
            )
            self.assertIn(
                "using tracer_core::core::dto::OperationAck;",
                actions[0].diff,
            )
