from __future__ import annotations

import json
from pathlib import Path
from tempfile import TemporaryDirectory
from unittest import TestCase

from tools.toolchain.commands.tidy.queue.source_cluster import (
    collect_source_clusters,
    resolve_source_cluster,
    resolve_task_source_cluster,
    source_file_key,
)
from tools.toolchain.commands.tidy.queue.task_model import (
    TaskDiagnostic,
    task_record_to_dict,
)

from ..support.tidy_task_model_support import _make_task_record


class TestTidySourceCluster(TestCase):
    def _write_task(
        self,
        tasks_dir: Path,
        *,
        cluster_id: str,
        task_id: str,
        source_file: str,
    ) -> Path:
        cluster_dir = tasks_dir / "clusters" / cluster_id
        cluster_dir.mkdir(parents=True, exist_ok=True)
        task_path = cluster_dir / f"task_{task_id}.json"
        diagnostics = (
            TaskDiagnostic(
                file=source_file,
                line=1,
                col=1,
                severity="warning",
                check="readability-identifier-naming",
                message="invalid case style for variable 'value'",
                raw_lines=(),
                notes=(),
            ),
        )
        record = _make_task_record(
            task_id=task_id,
            cluster_id=cluster_id,
            source_file=source_file,
            diagnostics=diagnostics,
            checks=("readability-identifier-naming",),
        )
        task_path.write_text(
            json.dumps(task_record_to_dict(record), indent=2),
            encoding="utf-8",
        )
        return task_path

    def test_collects_all_pending_tasks_for_each_source(self):
        with TemporaryDirectory() as temp_dir:
            tasks_dir = Path(temp_dir)
            source_a = str(tasks_dir / "src" / "cluster_a.cpp")
            source_b = str(tasks_dir / "src" / "cluster_b.cpp")
            self._write_task(
                tasks_dir,
                cluster_id="cluster_a",
                task_id="004",
                source_file=source_a,
            )
            self._write_task(
                tasks_dir,
                cluster_id="cluster_a",
                task_id="001",
                source_file=source_a,
            )
            self._write_task(
                tasks_dir,
                cluster_id="cluster_b",
                task_id="002",
                source_file=source_b,
            )

            clusters = collect_source_clusters(tasks_dir)

            self.assertEqual(len(clusters), 2)
            self.assertEqual(clusters[0].source_key, source_file_key(source_a))
            self.assertEqual(clusters[0].task_ids, ("001", "004"))
            self.assertEqual(clusters[0].cluster_id, "cluster_a")
            self.assertEqual(clusters[1].task_ids, ("002",))

    def test_collects_each_cluster_directory_without_batch_filtering(self):
        with TemporaryDirectory() as temp_dir:
            tasks_dir = Path(temp_dir)
            source_a = str(tasks_dir / "cluster_a.cpp")
            source_b = str(tasks_dir / "cluster_b.cpp")
            self._write_task(
                tasks_dir,
                cluster_id="cluster_a",
                task_id="001",
                source_file=source_a,
            )
            self._write_task(
                tasks_dir,
                cluster_id="cluster_b",
                task_id="002",
                source_file=source_b,
            )

            clusters = collect_source_clusters(tasks_dir)

            self.assertEqual([cluster.task_ids for cluster in clusters], [("001",), ("002",)])

    def test_resolves_current_cluster_after_task_queue_changes(self):
        with TemporaryDirectory() as temp_dir:
            tasks_dir = Path(temp_dir)
            source_file = str(tasks_dir / "cluster.cpp")
            task_path = self._write_task(
                tasks_dir,
                cluster_id="cluster_a",
                task_id="001",
                source_file=source_file,
            )
            self._write_task(
                tasks_dir,
                cluster_id="cluster_a",
                task_id="002",
                source_file=source_file,
            )

            by_source = resolve_source_cluster(tasks_dir, source_file=source_file)
            by_task = resolve_task_source_cluster(tasks_dir, task_path)

            self.assertIsNotNone(by_source)
            self.assertEqual(by_source, by_task)
            self.assertEqual(by_task.task_ids, ("001", "002"))
