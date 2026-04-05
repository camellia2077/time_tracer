from unittest import TestCase

from tools.toolchain.commands.tidy.tasking.task_model import (
    TaskDiagnostic,
    TaskSnippet,
    build_task_draft,
    finalize_task_record,
    render_text,
    render_toon,
    task_record_to_dict,
)

from ..support.tidy_task_model_support import _make_task_record


class TestTidyTaskRender(TestCase):
    def test_task_record_to_dict_writes_minimal_v2_schema(self):
        source_file = "C:/code/time_tracer/libs/tracer_core/src/infra/query/data/stats/stats_boundary.module.cpp"
        record = _make_task_record(
            source_file=source_file,
            diagnostics=(
                TaskDiagnostic(
                    file=source_file,
                    line=3,
                    col=8,
                    severity="error",
                    check="clang-diagnostic-error",
                    message="module not found",
                    raw_lines=(
                        f"{source_file}:3:8: error: module not found [clang-diagnostic-error]",
                        "    3 | module tracer.core.infrastructure.query.data.stats.boundary;",
                        "      | ~~~~~~~^~~~~~",
                    ),
                    notes=(),
                ),
            ),
            snippets=(
                TaskSnippet(
                    diagnostic_index=1,
                    source_line=3,
                    code="module tracer.core.infrastructure.query.data.stats.boundary;",
                    caret="~~~~~~~^~~~~~",
                    notes=(),
                ),
            ),
        )

        payload = task_record_to_dict(record)

        self.assertEqual(payload["version"], 3)
        self.assertEqual(payload["queue_batch_id"], "batch_001")
        self.assertIn("queue_generation", payload)
        self.assertNotIn("summary", payload)
        self.assertNotIn("snippets", payload)
        self.assertNotIn("raw_lines", payload)
        self.assertEqual(payload["compiler_errors"], True)
        self.assertEqual(payload["diagnostics"][0]["source_line"], 3)
        self.assertEqual(
            payload["diagnostics"][0]["code"],
            "module tracer.core.infrastructure.query.data.stats.boundary;",
        )
        self.assertEqual(payload["diagnostics"][0]["caret"], "~~~~~~~^~~~~~")
        self.assertNotIn("raw_lines", payload["diagnostics"][0])

    def test_render_text_keeps_caret_and_filters_noise(self):
        source_file = "C:/code/time_tracer/libs/tracer_core/src/application/importer/import_service.cpp"
        draft = build_task_draft(
            [
                (
                    f"{source_file}:2:8: error: module 'tracer.core.domain.model.daily_log' "
                    "not found [clang-diagnostic-error]"
                ),
                "    2 | import tracer.core.domain.model.daily_log;",
                "      | ~~~~~~~^~~~~~",
                "Suppressed 52637 warnings (52637 in non-user code).",
                "Use -header-filter=.* to display errors from all non-system headers.",
                "Found compiler error(s).",
            ]
        )

        self.assertIsNotNone(draft)
        record = finalize_task_record(
            draft,
            task_id="002",
            batch_id="batch_001",
            queue_generation=1,
            workspace="build_tidy_core_family",
            source_scope="core_family",
        )
        rendered = render_text(record)

        self.assertIn("File: C:/code/time_tracer/libs/tracer_core/src/application/importer/import_service.cpp", rendered)
        self.assertIn("error: module 'tracer.core.domain.model.daily_log' not found [clang-diagnostic-error]", rendered)
        self.assertIn("2 | import tracer.core.domain.model.daily_log;", rendered)
        self.assertIn("| ~~~~~~~^~~~~~", rendered)
        self.assertNotIn("Suppressed 52637 warnings", rendered)
        self.assertNotIn("Use -header-filter=.*", rendered)
        self.assertNotIn("Found compiler error(s).", rendered)

    def test_render_toon_outputs_compact_schema(self):
        source_file = "C:/code/time_tracer/libs/tracer_core/src/infra/query/data/stats/stats_boundary.module.cpp"
        record = _make_task_record(
            source_file=source_file,
            diagnostics=(
                TaskDiagnostic(
                    file=source_file,
                    line=3,
                    col=8,
                    severity="error",
                    check="clang-diagnostic-error",
                    message="module not found",
                    raw_lines=(
                        f"{source_file}:3:8: error: module not found [clang-diagnostic-error]",
                        "    3 | module tracer.core.infrastructure.query.data.stats.boundary;",
                        "      | ~~~~~~~^~~~~~",
                    ),
                    notes=(),
                ),
            ),
            snippets=(
                TaskSnippet(
                    diagnostic_index=1,
                    source_line=3,
                    code="module tracer.core.infrastructure.query.data.stats.boundary;",
                    caret="~~~~~~~^~~~~~",
                    notes=(),
                ),
            ),
        )

        rendered = render_toon(record)

        self.assertIn("task:", rendered)
        self.assertIn("diagnostics[1]{index,line,col,severity,check,message}:", rendered)
        self.assertIn("snippets[1]{diag,line,code,hint}:", rendered)
        self.assertIn("1,3,8,error,clang-diagnostic-error,module not found", rendered)
        self.assertIn(
            "1,3,module tracer.core.infrastructure.query.data.stats.boundary;,",
            rendered,
        )
        self.assertNotIn("~~~~~~~^~~~~~", rendered)

    def test_render_toon_keeps_semantic_hint_and_not_visual_marker(self):
        source_file = "C:/code/time_tracer/libs/tracer_core/src/infra/query/data/renderers/semantic_json_renderer.cpp"
        record = _make_task_record(
            source_file=source_file,
            diagnostics=(
                TaskDiagnostic(
                    file=source_file,
                    line=9,
                    col=4,
                    severity="warning",
                    check="readability-identifier-naming",
                    message="invalid case style for variable 'payload'",
                    raw_lines=(
                        f"{source_file}:9:4: warning: invalid case style for variable 'payload' [readability-identifier-naming]",
                        "    | kPayload",
                    ),
                    notes=(),
                ),
            ),
            snippets=(
                TaskSnippet(
                    diagnostic_index=1,
                    source_line=None,
                    code="",
                    caret="kPayload",
                    notes=(),
                ),
            ),
            checks=("readability-identifier-naming",),
        )

        rendered = render_toon(record)

        self.assertIn("snippets[1]{diag,line,code,hint}:", rendered)
        self.assertIn("1,,,kPayload", rendered)
