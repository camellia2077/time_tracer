import json
from pathlib import Path
from tempfile import TemporaryDirectory
from unittest import TestCase

from tools.toolchain.commands.tidy.tasking.task_builder import split_and_sort
from tools.toolchain.commands.tidy.tasking.task_log import load_task_record
from tools.toolchain.core.context import Context

from ..support.tidy_task_model_support import REPO_ROOT


class TestTidyTaskBuilderSplit(TestCase):
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
            queue_state = json.loads(
                (tasks_dir / "queue_state.json").read_text(encoding="utf-8")
            )
            self.assertEqual(queue_state["queue_generation"], 1)
            self.assertEqual(queue_state["task_count"], 1)
            self.assertEqual(queue_state["task_view"], "text")

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
