import json
from pathlib import Path
from tempfile import TemporaryDirectory
from unittest import TestCase

from tools.toolchain.commands.tidy.task_builder import split_and_sort
from tools.toolchain.commands.tidy.refresh_internal.refresh_mapper import collect_batch_files
from tools.toolchain.commands.tidy.task_log import (
    list_task_paths,
    load_task_record,
    resolve_task_json_path,
    resolve_task_log_path,
)
from tools.toolchain.commands.tidy.task_model import (
    TaskDiagnostic,
    TaskRecord,
    TaskSnippet,
    TaskSummary,
    TaskSummaryEntry,
    build_task_draft,
    finalize_task_record,
    render_text,
    render_toon,
    task_record_to_dict,
)
from tools.toolchain.core.context import Context
from tools.toolchain.services.rename_planner import collect_rename_candidates

REPO_ROOT = Path(__file__).resolve().parents[3]


def _make_task_record(
    *,
    task_id: str = "001",
    batch_id: str = "batch_001",
    source_file: str,
    diagnostics: tuple[TaskDiagnostic, ...],
    snippets: tuple[TaskSnippet, ...] = (),
    checks: tuple[str, ...] = ("clang-diagnostic-error",),
) -> TaskRecord:
    summary = TaskSummary(
        diagnostic_count=len(diagnostics),
        compiler_errors=any(item.severity == "error" for item in diagnostics),
        files=(TaskSummaryEntry(name=source_file, count=max(1, len(diagnostics))),),
        checks=tuple(TaskSummaryEntry(name=check, count=1) for check in checks),
    )
    return TaskRecord(
        version=2,
        task_id=task_id,
        batch_id=batch_id,
        source_file=source_file,
        workspace="build_tidy_core_family",
        source_scope="core_family",
        checks=checks,
        summary=summary,
        diagnostics=diagnostics,
        snippets=snippets,
        raw_lines=tuple(line for diagnostic in diagnostics for line in diagnostic.raw_lines),
    )


class TestTidyTaskModel(TestCase):
    def test_resolve_task_log_path_prefers_canonical_json(self):
        with TemporaryDirectory() as temp_dir:
            tasks_dir = Path(temp_dir)
            batch_dir = tasks_dir / "batch_001"
            batch_dir.mkdir(parents=True)
            source_file = str(tasks_dir / "demo.cpp")
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
                        ),
                        notes=(),
                    ),
                ),
            )
            json_path = batch_dir / "task_001.json"
            json_path.write_text(json.dumps(task_record_to_dict(record), indent=2), encoding="utf-8")
            (batch_dir / "task_001.log").write_text("legacy log\n", encoding="utf-8")

            resolved = resolve_task_log_path(tasks_dir)

            self.assertEqual(resolved, json_path)
            loaded = load_task_record(resolved)
            self.assertEqual(loaded.task_id, "001")
            self.assertEqual(loaded.source_file, source_file)

    def test_resolve_task_json_path_rejects_view_only_artifacts(self):
        with TemporaryDirectory() as temp_dir:
            tasks_dir = Path(temp_dir)
            batch_dir = tasks_dir / "batch_001"
            batch_dir.mkdir(parents=True)
            toon_path = batch_dir / "task_001.toon"
            toon_path.write_text("task:\n", encoding="utf-8")

            with self.assertRaises(FileNotFoundError):
                resolve_task_json_path(tasks_dir, task_log_path=str(toon_path))

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

        self.assertEqual(payload["version"], 2)
        self.assertEqual(payload["queue_batch_id"], "batch_001")
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

    def test_list_task_paths_keeps_legacy_logs_without_json(self):
        with TemporaryDirectory() as temp_dir:
            tasks_dir = Path(temp_dir)
            batch_one = tasks_dir / "batch_001"
            batch_two = tasks_dir / "batch_002"
            batch_one.mkdir(parents=True)
            batch_two.mkdir(parents=True)

            source_file = str(tasks_dir / "demo.cpp")
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
                        ),
                        notes=(),
                    ),
                ),
            )
            (batch_one / "task_001.json").write_text(
                json.dumps(task_record_to_dict(record), indent=2),
                encoding="utf-8",
            )
            (batch_one / "task_001.log").write_text("legacy duplicate\n", encoding="utf-8")
            (batch_two / "task_002.log").write_text(
                "\n".join(
                    [
                        f"File: {source_file}",
                        f"{source_file}:9:4: warning: invalid case style for variable 'payload' [readability-identifier-naming]",
                    ]
                )
                + "\n",
                encoding="utf-8",
            )

            task_paths = list_task_paths(tasks_dir)

            self.assertEqual(
                task_paths,
                [
                    batch_one / "task_001.json",
                    batch_two / "task_002.log",
                ],
            )

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

    def test_collect_batch_files_reads_canonical_json(self):
        with TemporaryDirectory() as temp_dir:
            tasks_dir = Path(temp_dir)
            batch_dir = tasks_dir / "batch_001"
            batch_dir.mkdir(parents=True)
            source_file = str(tasks_dir / "primary.cpp")
            header_file = str(tasks_dir / "detail.hpp")
            record = _make_task_record(
                source_file=source_file,
                diagnostics=(
                    TaskDiagnostic(
                        file=header_file,
                        line=7,
                        col=4,
                        severity="warning",
                        check="readability-identifier-naming",
                        message="invalid case style for variable 'payload'",
                        raw_lines=(
                            f"{header_file}:7:4: warning: invalid case style for variable 'payload' [readability-identifier-naming]",
                            "    | kPayload",
                        ),
                        notes=(),
                    ),
                ),
                checks=("readability-identifier-naming",),
            )
            (batch_dir / "task_001.json").write_text(
                json.dumps(task_record_to_dict(record), indent=2),
                encoding="utf-8",
            )

            touched = collect_batch_files(batch_dir)

            self.assertEqual(
                [str(path) for path in touched],
                sorted([header_file, source_file], key=lambda item: item.lower()),
            )

    def test_collect_rename_candidates_reads_json_records(self):
        with TemporaryDirectory() as temp_dir:
            tasks_dir = Path(temp_dir)
            batch_dir = tasks_dir / "batch_001"
            batch_dir.mkdir(parents=True)
            source_file = str(tasks_dir / "example.cpp")
            record = _make_task_record(
                source_file=source_file,
                diagnostics=(
                    TaskDiagnostic(
                        file=source_file,
                        line=3,
                        col=9,
                        severity="warning",
                        check="readability-identifier-naming",
                        message="invalid case style for variable 'payload'",
                        raw_lines=(
                            f"{source_file}:3:9: warning: invalid case style for variable 'payload' [readability-identifier-naming]",
                            "    | kPayload",
                        ),
                        notes=(),
                    ),
                ),
                checks=("readability-identifier-naming",),
            )
            (batch_dir / "task_001.json").write_text(
                json.dumps(task_record_to_dict(record), indent=2),
                encoding="utf-8",
            )

            candidates = collect_rename_candidates(
                tasks_dir=tasks_dir,
                check_name="readability-identifier-naming",
                allowed_kinds=["variable"],
            )

            self.assertEqual(len(candidates), 1)
            self.assertEqual(candidates[0]["new_name"], "kPayload")

    def test_collect_rename_candidates_reads_toon_records(self):
        with TemporaryDirectory() as temp_dir:
            tasks_dir = Path(temp_dir)
            batch_dir = tasks_dir / "batch_001"
            batch_dir.mkdir(parents=True)
            source_file = str(tasks_dir / "example.cpp")
            record = _make_task_record(
                source_file=source_file,
                diagnostics=(
                    TaskDiagnostic(
                        file=source_file,
                        line=3,
                        col=9,
                        severity="warning",
                        check="readability-identifier-naming",
                        message="invalid case style for variable 'payload'",
                        raw_lines=(
                            f"{source_file}:3:9: warning: invalid case style for variable 'payload' [readability-identifier-naming]",
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
            (batch_dir / "task_001.toon").write_text(
                render_toon(record),
                encoding="utf-8",
            )

            candidates = collect_rename_candidates(
                tasks_dir=tasks_dir,
                check_name="readability-identifier-naming",
                allowed_kinds=["variable"],
            )

            self.assertEqual(len(candidates), 1)
            self.assertEqual(candidates[0]["new_name"], "kPayload")

    def test_split_and_sort_writes_text_only_view(self):
        with TemporaryDirectory() as temp_dir:
            tasks_dir = Path(temp_dir)
            ctx = Context(REPO_ROOT)
            stats = split_and_sort(
                ctx,
                "\n".join(
                    [
                        "[1/1] Building CXX object",
                        (
                            "C:/code/time_tracer/libs/tracer_core/src/infra/query/data/stats/"
                            "stats_boundary.module.cpp:3:8: error: module not found "
                            "[clang-diagnostic-error]"
                        ),
                        "    3 | module tracer.core.infrastructure.query.data.stats.boundary;",
                        "      | ~~~~~~~^~~~~~",
                    ]
                ),
                tasks_dir,
                task_view="text",
                workspace_name="build_tidy_core_family",
                source_scope="core_family",
            )

            self.assertEqual(stats["tasks"], 1)
            self.assertTrue((tasks_dir / "batch_001" / "task_001.json").exists())
            self.assertTrue((tasks_dir / "batch_001" / "task_001.log").exists())
            self.assertFalse((tasks_dir / "batch_001" / "task_001.toon").exists())

    def test_split_and_sort_writes_json_only_view(self):
        with TemporaryDirectory() as temp_dir:
            tasks_dir = Path(temp_dir)
            ctx = Context(REPO_ROOT)
            split_and_sort(
                ctx,
                "\n".join(
                    [
                        "[1/1] Building CXX object",
                        "C:/repo/example.cpp:7:4: warning: invalid case style for variable 'payload' [readability-identifier-naming]",
                        "    | kPayload",
                    ]
                ),
                tasks_dir,
                task_view="json",
                workspace_name="build_tidy_core_family",
                source_scope="core_family",
            )

            self.assertTrue((tasks_dir / "batch_001" / "task_001.json").exists())
            self.assertFalse((tasks_dir / "batch_001" / "task_001.log").exists())
            self.assertFalse((tasks_dir / "batch_001" / "task_001.toon").exists())

    def test_split_and_sort_writes_optional_toon_view(self):
        with TemporaryDirectory() as temp_dir:
            tasks_dir = Path(temp_dir)
            ctx = Context(REPO_ROOT)
            split_and_sort(
                ctx,
                "\n".join(
                    [
                        "[1/1] Building CXX object",
                        "C:/repo/example.cpp:7:4: warning: invalid case style for variable 'payload' [readability-identifier-naming]",
                        "    | kPayload",
                    ]
                ),
                tasks_dir,
                task_view="text+toon",
                workspace_name="build_tidy_core_family",
                source_scope="core_family",
            )

            self.assertTrue((tasks_dir / "batch_001" / "task_001.json").exists())
            self.assertTrue((tasks_dir / "batch_001" / "task_001.log").exists())
            self.assertTrue((tasks_dir / "batch_001" / "task_001.toon").exists())

    def test_split_and_sort_writes_toon_only_view(self):
        with TemporaryDirectory() as temp_dir:
            tasks_dir = Path(temp_dir)
            ctx = Context(REPO_ROOT)
            split_and_sort(
                ctx,
                "\n".join(
                    [
                        "[1/1] Building CXX object",
                        "C:/repo/example.cpp:7:4: warning: invalid case style for variable 'payload' [readability-identifier-naming]",
                        "    | kPayload",
                    ]
                ),
                tasks_dir,
                task_view="toon",
                workspace_name="build_tidy_core_family",
                source_scope="core_family",
            )

            self.assertTrue((tasks_dir / "batch_001" / "task_001.json").exists())
            self.assertFalse((tasks_dir / "batch_001" / "task_001.log").exists())
            self.assertTrue((tasks_dir / "batch_001" / "task_001.toon").exists())

    def test_split_and_sort_reuses_existing_task_view_when_unspecified(self):
        with TemporaryDirectory() as temp_dir:
            tasks_dir = Path(temp_dir)
            batch_dir = tasks_dir / "batch_003"
            batch_dir.mkdir(parents=True)
            (batch_dir / "task_021.toon").write_text("task:\n", encoding="utf-8")
            ctx = Context(REPO_ROOT)

            split_and_sort(
                ctx,
                "\n".join(
                    [
                        "[1/1] Building CXX object",
                        "C:/repo/example.cpp:7:4: warning: invalid case style for variable 'payload' [readability-identifier-naming]",
                        "    | kPayload",
                    ]
                ),
                tasks_dir,
                task_view=None,
                workspace_name="build_tidy_core_family",
                source_scope="core_family",
            )

            self.assertTrue((tasks_dir / "batch_003" / "task_021.json").exists())
            self.assertFalse((tasks_dir / "batch_003" / "task_021.log").exists())
            self.assertTrue((tasks_dir / "batch_003" / "task_021.toon").exists())

    def test_split_and_sort_continues_existing_queue_namespace(self):
        with TemporaryDirectory() as temp_dir:
            tasks_dir = Path(temp_dir)
            batch_dir = tasks_dir / "batch_003"
            batch_dir.mkdir(parents=True)
            (batch_dir / "task_021.json").write_text("{}", encoding="utf-8")
            ctx = Context(REPO_ROOT)

            split_and_sort(
                ctx,
                "\n".join(
                    [
                        "[1/2] Building CXX object",
                        "C:/repo/example_a.cpp:7:4: warning: invalid case style for variable 'payload_a' [readability-identifier-naming]",
                        "    | kPayloadA",
                        "[2/2] Building CXX object",
                        "C:/repo/example_b.cpp:9:4: warning: invalid case style for variable 'payload_b' [readability-identifier-naming]",
                        "    | kPayloadB",
                    ]
                ),
                tasks_dir,
                batch_size=1,
                task_view="toon",
                workspace_name="build_tidy_core_family",
                source_scope="core_family",
            )

            self.assertTrue((tasks_dir / "batch_003" / "task_021.json").exists())
            self.assertTrue((tasks_dir / "batch_003" / "task_021.toon").exists())
            self.assertTrue((tasks_dir / "batch_004" / "task_022.json").exists())
            self.assertTrue((tasks_dir / "batch_004" / "task_022.toon").exists())
            self.assertFalse((tasks_dir / "batch_001").exists())

    def test_split_and_sort_filters_tasks_not_present_in_compile_db(self):
        with TemporaryDirectory() as temp_dir:
            tasks_dir = Path(temp_dir)
            ctx = Context(REPO_ROOT)

            stats = split_and_sort(
                ctx,
                "\n".join(
                    [
                        "[1/2] Building CXX object",
                        "C:/repo/kept.cpp:7:4: warning: invalid case style for variable 'payload' [readability-identifier-naming]",
                        "    | kPayload",
                        "[2/2] Building CXX object",
                        "C:/repo/orphan.module.cpp:9:8: error: module not found [clang-diagnostic-error]",
                        "    9 | import tracer.core.application.pipeline.orchestrator;",
                        "      | ~~~~~~~^~~~~~",
                    ]
                ),
                tasks_dir,
                task_view="toon",
                workspace_name="build_tidy_core_family",
                source_scope="core_family",
                compile_units=[Path("C:/repo/kept.cpp")],
            )

            self.assertEqual(stats["tasks"], 1)
            self.assertTrue((tasks_dir / "batch_001" / "task_001.json").exists())
            loaded = load_task_record(tasks_dir / "batch_001" / "task_001.json")
            self.assertEqual(loaded.source_file, "C:/repo/kept.cpp")
