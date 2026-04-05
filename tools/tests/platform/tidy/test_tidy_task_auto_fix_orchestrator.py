from tempfile import TemporaryDirectory
from unittest import TestCase
from unittest.mock import patch

from tools.tests.platform.support.tidy_task_auto_fix_support import AutoFixFixtureBuilder, DiagnosticEntry
from tools.tests.platform.support.path_assertions import assert_same_path
from tools.toolchain.commands.tidy.autofix.models import ExecutionRecord
from tools.toolchain.commands.tidy.tasking.task_auto_fix import run_task_auto_fix


class TestTidyTaskAutoFixOrchestrator(TestCase):
    def test_run_task_auto_fix_aggregates_mixed_task_actions(self):
        with TemporaryDirectory() as temp_dir:
            fb = AutoFixFixtureBuilder(temp_dir)
            fb.write_source(
                "libs/tracer_core/src/application/use_cases/query_api.hpp",
                [
                    "#pragma once",
                    "namespace tracer::core::application::use_cases {",
                    "auto RunTreeQuery(const tracer_core::core::dto::TreeQueryRequest& request)",
                    "    -> tracer_core::core::dto::TreeQueryResponse;",
                    "}  // namespace tracer::core::application::use_cases",
                ],
            )
            source_file = fb.write_source(
                "libs/tracer_core/src/application/use_cases/query_api.cpp",
                [
                    '#include "application/use_cases/query_api.hpp"',
                    "",
                    "namespace tracer::core::application::use_cases {",
                    "",
                    "using namespace tracer_core::core::dto;",
                    "",
                    "auto RunTreeQuery(const TreeQueryRequest& request) -> TreeQueryResponse {",
                    "  (void)request;",
                    "  const auto tree_result = BuildTree();",
                    "  return {.ok = true, .tree = TreeQueryPayload{.nodes = std::move(*tree_result)}};",
                    "}",
                    "",
                    "}  // namespace tracer::core::application::use_cases",
                ],
            )
            task_path, _json_path = fb.write_batch_task_artifacts(
                batch_id="batch_003",
                task_id="030",
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
                    ),
                    DiagnosticEntry(
                        line=9,
                        col=14,
                        check="readability-identifier-naming",
                        message="invalid case style for constant 'tree_result'",
                        raw_lines=(
                            (
                                f"{source_file}:9:14: warning: invalid case style for "
                                "constant 'tree_result' [readability-identifier-naming]"
                            ),
                            "    | kTreeResult",
                        ),
                    ),
                ],
            )

            def _mock_rename_execute(_context, intents):
                return [
                    ExecutionRecord(
                        intent_id=intents[0].intent_id,
                        status="previewed",
                        reason="supported_rule_driven_const_rename",
                        edit_count=2,
                        changed_files=(str(source_file),),
                        old_name="tree_result",
                        new_name="kTreeResult",
                    )
                ]

            with (
                patch(
                    "tools.toolchain.commands.tidy.autofix.orchestrator.resolve_workspace",
                    return_value=fb.workspace(),
                ),
                patch(
                    "tools.toolchain.commands.tidy.autofix.orchestrator.ClangdRenameEngine.execute",
                    side_effect=_mock_rename_execute,
                ),
            ):
                result = run_task_auto_fix(
                    fb.context(),
                    task_log_path=str(task_path),
                    dry_run=True,
                    report_suffix="patch",
                )

            self.assertEqual(result.action_count, 2)
            self.assertEqual(result.previewed, 2)
            self.assertEqual(result.failed, 0)
            self.assertEqual(result.skipped, 0)
            assert_same_path(result.task_log, task_path.with_suffix(".json"))
            self.assertEqual({action.kind for action in result.actions}, {"rename", "using_namespace"})
            rename_action = next(action for action in result.actions if action.kind == "rename")
            self.assertEqual(rename_action.new_name, "kTreeResult")
            using_action = next(action for action in result.actions if action.kind == "using_namespace")
            self.assertIn("using tracer_core::core::dto::TreeQueryRequest;", using_action.replacement)

    def test_run_task_auto_fix_applies_concise_preprocessor_directive(self):
        with TemporaryDirectory() as temp_dir:
            fb = AutoFixFixtureBuilder(temp_dir)
            source_file = fb.write_source(
                "libs/tracer_core/src/application/use_cases/report_guard.cpp",
                [
                    "#if defined(_WIN32)",
                    "void UseWinApi();",
                    "#endif",
                ],
            )
            task_path, _json_path = fb.write_batch_task_artifacts(
                batch_id="batch_004",
                task_id="031",
                source_file=source_file,
                diagnostics=[
                    DiagnosticEntry(
                        line=1,
                        col=2,
                        check="readability-use-concise-preprocessor-directives",
                        message=(
                            "preprocessor condition can be written more concisely using "
                            "'#ifdef'"
                        ),
                    )
                ],
            )

            with patch(
                "tools.toolchain.commands.tidy.autofix.orchestrator.resolve_workspace",
                return_value=fb.workspace(),
            ):
                result = run_task_auto_fix(
                    fb.context(),
                    task_log_path=str(task_path),
                    dry_run=False,
                    report_suffix="fix",
                )

            self.assertEqual(result.action_count, 1)
            self.assertEqual(result.applied, 1)
            self.assertEqual(result.failed, 0)
            self.assertEqual(result.skipped, 0)
            action = result.actions[0]
            self.assertEqual(action.kind, "concise_preprocessor_directive")
            self.assertEqual(action.reason, "concise_preprocessor_directive_rewritten")
            updated = source_file.read_text(encoding="utf-8")
            self.assertIn("#ifdef _WIN32", updated)
            self.assertNotIn("#if defined(_WIN32)", updated)
