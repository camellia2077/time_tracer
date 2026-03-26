import json
from types import SimpleNamespace
from pathlib import Path
from tempfile import TemporaryDirectory
from unittest import TestCase
from unittest.mock import patch

from tools.toolchain.commands.tidy.task_auto_fix import (
    apply_explicit_constructor_actions,
    apply_runtime_int_actions,
    apply_using_namespace_actions,
    plan_explicit_constructor_actions,
    plan_runtime_int_actions,
    plan_redundant_cast_actions,
    plan_using_namespace_actions,
    rename_candidates,
    run_task_auto_fix,
    suggest_const_name,
    suggest_task_refactors,
)
from tools.toolchain.commands.tidy.autofix.models import ExecutionRecord
from tools.toolchain.commands.tidy.task_log import parse_task_log, resolve_task_log_path
from tools.toolchain.commands.tidy.task_model import (
    TaskDiagnostic,
    TaskRecord,
    TaskSummary,
    TaskSummaryEntry,
    task_record_to_dict,
)
from tools.toolchain.commands.tidy.workspace import ResolvedTidyWorkspace


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

    def test_plan_runtime_int_actions_extracts_replacement(self):
        with TemporaryDirectory() as temp_dir:
            temp_root = Path(temp_dir)
            source_file = temp_root / "example.cpp"
            source_file.write_text(
                "\n".join(
                    [
                        '#include "example.hpp"',
                        "",
                        "long long duration = 0;",
                    ]
                )
                + "\n",
                encoding="utf-8",
            )
            task_log = temp_root / "task_003.toon"
            task_log.write_text(
                "\n".join(
                    [
                        "task:",
                        "  id: 003",
                        "  batch: batch_001",
                        f"  source: {source_file}",
                        "summary:",
                        "  diagnostics: 1",
                        "  compiler_errors: false",
                        "checks[1]{name,count}:",
                        "  google-runtime-int,1",
                        "diagnostics[1]{index,line,col,severity,check,message}:",
                        "  1,3,1,warning,google-runtime-int,consider replacing 'long long' with 'int64'",
                    ]
                ),
                encoding="utf-8",
            )

            parsed = parse_task_log(task_log)
            actions = plan_runtime_int_actions(parsed)

            self.assertEqual(len(actions), 1)
            self.assertEqual(actions[0].kind, "runtime_int")
            self.assertEqual(actions[0].old_name, "long long")
            self.assertEqual(actions[0].new_name, "std::int64_t")
            self.assertEqual(actions[0].line, 3)

    def test_rename_candidates_fall_back_to_rule_template_for_toon_tasks(self):
        with TemporaryDirectory() as temp_dir:
            source_dir = Path(temp_dir) / "libs" / "tracer_adapters_io" / "src" / "infra" / "io" / "core"
            source_dir.mkdir(parents=True)
            source_file = source_dir / "file_writer.cpp"
            source_file.write_text(
                "\n".join(
                    [
                        "void FileWriter::WriteCanonicalText() {",
                        "  const std::string canonical = BuildCanonical();",
                        "  const auto bytes = ToUtf8Bytes(canonical);",
                        "}",
                    ]
                )
                + "\n",
                encoding="utf-8",
            )
            task_log = Path(temp_dir) / "task_029.toon"
            task_log.write_text(
                "\n".join(
                    [
                        "task:",
                        "  id: 029",
                        "  batch: batch_003",
                        f"  source: {source_file}",
                        "summary:",
                        "  diagnostics: 2",
                        "  compiler_errors: false",
                        "checks[1]{name,count}:",
                        "  readability-identifier-naming,2",
                        "diagnostics[2]{index,line,col,severity,check,message}:",
                        "  1,2,21,warning,readability-identifier-naming,invalid case style for constant 'canonical'",
                        "  2,3,14,warning,readability-identifier-naming,invalid case style for constant 'bytes'",
                    ]
                ),
                encoding="utf-8",
            )

            parsed = parse_task_log(task_log)
            candidates = rename_candidates(parsed)

            self.assertEqual(len(candidates), 2)
            self.assertEqual(candidates[0]["new_name"], "kCanonical")
            self.assertEqual(candidates[1]["new_name"], "kBytes")

    def test_apply_runtime_int_actions_rewrites_type_and_adds_cstdint_include(self):
        with TemporaryDirectory() as temp_dir:
            temp_root = Path(temp_dir)
            source_file = temp_root / "example.cpp"
            source_file.write_text(
                "\n".join(
                    [
                        '// sample file',
                        '#include "example.hpp"',
                        "",
                        "long long duration = 0;",
                    ]
                )
                + "\n",
                encoding="utf-8",
            )
            task_log = temp_root / "task_004.log"
            task_log.write_text(
                "\n".join(
                    [
                        f"File: {source_file}",
                        "============================================================",
                        (
                            f"{source_file}:4:1: warning: consider replacing 'long long' with 'int64' "
                            "[google-runtime-int]"
                        ),
                    ]
                ),
                encoding="utf-8",
            )

            parsed = parse_task_log(task_log)
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
            temp_root = Path(temp_dir)
            source_file = temp_root / "text_validator.cpp"
            source_file.write_text(
                "\n".join(
                    [
                        "struct PImpl {",
                        "  PImpl(const ConverterConfig& config) : line_processor(config) {}",
                        "};",
                    ]
                )
                + "\n",
                encoding="utf-8",
            )
            task_log = temp_root / "task_018.toon"
            task_log.write_text(
                "\n".join(
                    [
                        "task:",
                        "  id: 018",
                        "  batch: batch_002",
                        f"  source: {source_file}",
                        "summary:",
                        "  diagnostics: 1",
                        "  compiler_errors: false",
                        "checks[1]{name,count}:",
                        "  google-explicit-constructor,1",
                        "diagnostics[1]{index,line,col,severity,check,message}:",
                        (
                            "  1,2,3,warning,google-explicit-constructor,"
                            "single-argument constructors must be marked explicit to avoid unintentional implicit conversions"
                        ),
                    ]
                ),
                encoding="utf-8",
            )

            parsed = parse_task_log(task_log)
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
            temp_root = Path(temp_dir) / "libs" / "tracer_core" / "src"
            source_dir = temp_root / "application" / "use_cases"
            source_dir.mkdir(parents=True)

            header_file = source_dir / "pipeline_api.hpp"
            header_file.write_text(
                "\n".join(
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
                    ]
                )
                + "\n",
                encoding="utf-8",
            )
            source_file = source_dir / "pipeline_api.cpp"
            source_file.write_text(
                "\n".join(
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
                    ]
                )
                + "\n",
                encoding="utf-8",
            )
            task_log = temp_root / "task_005.toon"
            task_log.write_text(
                "\n".join(
                    [
                        "task:",
                        "  id: 005",
                        "  batch: batch_001",
                        f"  source: {source_file}",
                        "summary:",
                        "  diagnostics: 1",
                        "  compiler_errors: false",
                        "checks[1]{name,count}:",
                        "  google-build-using-namespace,1",
                        "diagnostics[1]{index,line,col,severity,check,message}:",
                        (
                            "  1,5,1,warning,google-build-using-namespace,"
                            "do not use namespace using-directives; use using-declarations instead"
                        ),
                    ]
                ),
                encoding="utf-8",
            )

            parsed = parse_task_log(task_log)
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

    def test_plan_using_namespace_actions_collects_symbols_from_dto_headers(self):
        with TemporaryDirectory() as temp_dir:
            temp_root = Path(temp_dir) / "libs" / "tracer_core" / "src"
            dto_dir = temp_root / "application" / "dto"
            use_cases_dir = temp_root / "application" / "use_cases"
            dto_dir.mkdir(parents=True)
            use_cases_dir.mkdir(parents=True)

            (dto_dir / "reporting_responses.hpp").write_text(
                "\n".join(
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
                    ]
                )
                + "\n",
                encoding="utf-8",
            )
            (dto_dir / "shared_envelopes.hpp").write_text(
                "\n".join(
                    [
                        "#ifndef SHARED_ENVELOPES_HPP_",
                        "#define SHARED_ENVELOPES_HPP_",
                        "namespace tracer_core::core::dto {",
                        "struct TextOutput {",
                        "  bool ok = true;",
                        "};",
                        "}  // namespace tracer_core::core::dto",
                    ]
                )
                + "\n",
                encoding="utf-8",
            )
            header_file = use_cases_dir / "helper.hpp"
            header_file.write_text(
                "\n".join(
                    [
                        '#include "application/dto/reporting_responses.hpp"',
                        '#include "application/dto/shared_envelopes.hpp"',
                        "namespace tracer::core::application::use_cases {",
                        "auto FormatStructured(",
                        "    const tracer_core::core::dto::StructuredReportOutput& output)",
                        "    -> tracer_core::core::dto::TextOutput;",
                        "}  // namespace tracer::core::application::use_cases",
                    ]
                )
                + "\n",
                encoding="utf-8",
            )
            source_file = use_cases_dir / "helper.cpp"
            source_file.write_text(
                "\n".join(
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
                    ]
                )
                + "\n",
                encoding="utf-8",
            )
            task_log = temp_root / "task_006.log"
            task_log.write_text(
                "\n".join(
                    [
                        f"File: {source_file}",
                        "============================================================",
                        (
                            f"{source_file}:5:1: warning: do not use namespace using-directives; "
                            "use using-declarations instead [google-build-using-namespace]"
                        ),
                    ]
                ),
                encoding="utf-8",
            )

            parsed = parse_task_log(task_log)
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

    def test_run_task_auto_fix_aggregates_mixed_task_actions(self):
        with TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            tasks_dir = root / "tasks"
            automation_dir = root / "automation"
            batch_dir = tasks_dir / "batch_003"
            source_dir = root / "libs" / "tracer_core" / "src" / "application" / "use_cases"
            batch_dir.mkdir(parents=True)
            automation_dir.mkdir(parents=True)
            source_dir.mkdir(parents=True)

            header_file = source_dir / "query_api.hpp"
            header_file.write_text(
                "\n".join(
                    [
                        "#pragma once",
                        "namespace tracer::core::application::use_cases {",
                        "auto RunTreeQuery(const tracer_core::core::dto::TreeQueryRequest& request)",
                        "    -> tracer_core::core::dto::TreeQueryResponse;",
                        "}  // namespace tracer::core::application::use_cases",
                    ]
                )
                + "\n",
                encoding="utf-8",
            )
            source_file = source_dir / "query_api.cpp"
            source_file.write_text(
                "\n".join(
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
                    ]
                )
                + "\n",
                encoding="utf-8",
            )
            task_path = batch_dir / "task_030.toon"
            task_path.write_text(
                "\n".join(
                    [
                        "task:",
                        "  id: 030",
                        "  batch: batch_003",
                        f"  source: {source_file}",
                        "summary:",
                        "  diagnostics: 2",
                        "  compiler_errors: false",
                        "checks[2]{name,count}:",
                        "  google-build-using-namespace,1",
                        "  readability-identifier-naming,1",
                        "diagnostics[2]{index,line,col,severity,check,message}:",
                        "  1,5,1,warning,google-build-using-namespace,do not use namespace using-directives; use using-declarations instead",
                        "  2,9,14,warning,readability-identifier-naming,invalid case style for constant 'tree_result'",
                    ]
                ),
                encoding="utf-8",
            )
            record = TaskRecord(
                version=2,
                task_id="030",
                batch_id="batch_003",
                source_file=str(source_file),
                workspace="build_tidy_test",
                source_scope=None,
                checks=("google-build-using-namespace", "readability-identifier-naming"),
                summary=TaskSummary(
                    diagnostic_count=2,
                    compiler_errors=False,
                    files=(TaskSummaryEntry(name=str(source_file), count=2),),
                    checks=(
                        TaskSummaryEntry(name="google-build-using-namespace", count=1),
                        TaskSummaryEntry(name="readability-identifier-naming", count=1),
                    ),
                ),
                diagnostics=(
                    TaskDiagnostic(
                        file=str(source_file),
                        line=5,
                        col=1,
                        severity="warning",
                        check="google-build-using-namespace",
                        message="do not use namespace using-directives; use using-declarations instead",
                        raw_lines=(
                            f"{source_file}:5:1: warning: do not use namespace using-directives; use using-declarations instead [google-build-using-namespace]",
                        ),
                        notes=(),
                    ),
                    TaskDiagnostic(
                        file=str(source_file),
                        line=9,
                        col=14,
                        severity="warning",
                        check="readability-identifier-naming",
                        message="invalid case style for constant 'tree_result'",
                        raw_lines=(
                            f"{source_file}:9:14: warning: invalid case style for constant 'tree_result' [readability-identifier-naming]",
                            "    | kTreeResult",
                        ),
                        notes=(),
                    ),
                ),
                snippets=(),
                raw_lines=(),
            )
            task_path.with_suffix(".json").write_text(
                json.dumps(task_record_to_dict(record), indent=2),
                encoding="utf-8",
            )

            ctx = SimpleNamespace()
            ctx.get_tidy_layout = lambda *_args, **_kwargs: SimpleNamespace(
                root=root,
                tasks_dir=tasks_dir,
                automation_dir=automation_dir,
                tasks_done_dir=root / "tasks_done",
                batch_state_path=root / "batch_state.json",
            )

            workspace = ResolvedTidyWorkspace(
                source_scope=None,
                build_dir_name="build_tidy_test",
                source_roots=[],
                prebuild_targets=[],
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
                patch("tools.toolchain.commands.tidy.autofix.orchestrator.resolve_workspace", return_value=workspace),
                patch("tools.toolchain.commands.tidy.autofix.orchestrator.ClangdRenameEngine.execute", side_effect=_mock_rename_execute),
            ):
                result = run_task_auto_fix(
                    ctx,
                    app_name="tracer_core_shell",
                    task_log_path=str(task_path),
                    tidy_build_dir_name="build_tidy_test",
                    dry_run=True,
                    report_suffix="patch",
                )

            self.assertEqual(result.action_count, 2)
            self.assertEqual(result.previewed, 2)
            self.assertEqual(result.failed, 0)
            self.assertEqual(result.skipped, 0)
            self.assertEqual(result.task_log, str(task_path.with_suffix(".json")))
            self.assertEqual({action.kind for action in result.actions}, {"rename", "using_namespace"})
            rename_action = next(action for action in result.actions if action.kind == "rename")
            self.assertEqual(rename_action.new_name, "kTreeResult")
            using_action = next(action for action in result.actions if action.kind == "using_namespace")
            self.assertIn("using tracer_core::core::dto::TreeQueryRequest;", using_action.replacement)

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
