from __future__ import annotations

import time
from pathlib import Path

from ....core.context import Context
from ...shared import tidy as tidy_shared
from .source_step import TidySourceStepCommand
from ..queue.source_cluster import collect_source_clusters
from ..workspace import resolve_workspace


class TidyAgentRunCommand:
    """Run bounded source-cluster steps and leave a resumable checkpoint."""

    def __init__(self, ctx: Context):
        self.ctx = ctx

    def execute(
        self,
        *,
        app_name: str,
        source_scope: str | None,
        tidy_build_dir_name: str | None,
        max_clusters: int = 3,
        max_tasks: int = 10,
        max_minutes: int = 30,
        verify_build_dir_name: str | None = None,
        profile_name: str | None = None,
        concise: bool = False,
        kill_build_procs: bool = False,
        strict: bool = False,
        config_file: str | None = None,
        strict_config: bool = False,
        dry_run: bool = False,
    ) -> int:
        if max_clusters <= 0:
            raise ValueError("--max-clusters must be > 0")
        if max_tasks <= 0:
            raise ValueError("--max-tasks must be > 0")
        if max_minutes <= 0:
            raise ValueError("--max-minutes must be > 0")

        workspace = resolve_workspace(
            self.ctx,
            build_dir_name=tidy_build_dir_name,
            source_scope=source_scope,
        )
        layout = self.ctx.get_tidy_layout(app_name, workspace.build_dir_name)
        tasks_dir = layout.tasks_dir
        started_at = time.monotonic()
        deadline = started_at + max_minutes * 60
        processed_clusters = 0
        processed_tasks = 0
        last_source_file: str | None = None
        status = "paused"
        reason = "slice_limit"
        exit_code = 0
        error: str | None = None

        while True:
            clusters = collect_source_clusters(tasks_dir)
            if not clusters:
                status = "paused"
                reason = "queue_empty_requires_tidy_close"
                break
            if processed_clusters >= max_clusters:
                reason = "max_clusters"
                break
            if time.monotonic() >= deadline:
                reason = "max_minutes"
                break

            cluster = clusters[0]
            cluster_task_count = len(cluster.task_paths)
            if processed_tasks and processed_tasks + cluster_task_count > max_tasks:
                reason = "max_tasks"
                break

            print(
                f"--- tidy-agent: processing cluster {processed_clusters + 1}/"
                f"{max_clusters}: source={cluster.source_file}, "
                f"tasks={','.join(cluster.task_ids)}"
            )
            ret = TidySourceStepCommand(self.ctx).execute(
                task_log_path=str(cluster.task_paths[0]),
                verify_build_dir_name=verify_build_dir_name,
                profile_name=profile_name,
                concise=concise,
                kill_build_procs=kill_build_procs,
                strict=strict,
                dry_run=dry_run,
                config_file=config_file,
                strict_config=strict_config,
            )
            last_source_file = cluster.source_file
            if dry_run:
                if ret == 0:
                    status = "previewed"
                    reason = "dry_run"
                    exit_code = 0
                elif ret == 2:
                    status = "blocked"
                    reason = "source_cluster_requires_manual_action_or_refresh"
                    exit_code = 2
                else:
                    status = "failed"
                    reason = "source_cluster_step_failed"
                    exit_code = ret
                    error = f"tidy-source-step exited with {ret}"
                break
            if ret == 0:
                processed_clusters += 1
                processed_tasks += cluster_task_count
                continue
            if ret == 2:
                status = "blocked"
                reason = "source_cluster_requires_manual_action_or_refresh"
                exit_code = 2
                break
            status = "failed"
            reason = "source_cluster_step_failed"
            exit_code = ret
            error = f"tidy-source-step exited with {ret}"
            break

        queue_head = self._queue_head(tasks_dir)
        state_path = layout.automation_dir / "agent_run_state.json"
        remaining_clusters = collect_source_clusters(tasks_dir) if tasks_dir.exists() else ()
        remaining_tasks = sum(len(cluster.task_paths) for cluster in remaining_clusters)
        tidy_shared.write_json_dict(
            state_path,
            {
                "version": 1,
                "dry_run": dry_run,
                "status": status,
                "reason": reason,
                "exit_code": exit_code,
                "app": app_name,
                "source_scope": workspace.source_scope,
                "tidy_build_dir": workspace.build_dir_name,
                "max_clusters": max_clusters,
                "max_tasks": max_tasks,
                "max_minutes": max_minutes,
                "processed_clusters": processed_clusters,
                "processed_tasks": processed_tasks,
                "last_source_file": last_source_file,
                "queue_head": queue_head,
                "remaining_tasks": remaining_tasks,
                "remaining_clusters": len(remaining_clusters),
                "next_action": self._next_action(status, reason, queue_head),
                "error": error,
            },
        )
        print(f"--- tidy-agent: status={status}, reason={reason}, state={state_path}")
        return exit_code

    @staticmethod
    def _queue_head(tasks_dir: Path) -> dict | None:
        clusters = collect_source_clusters(tasks_dir)
        if not clusters:
            return None
        cluster = clusters[0]
        record = cluster.task_records[0]
        return {
            "task_id": record.task_id,
            "cluster_id": cluster.cluster_id,
            "source_file": cluster.source_file,
            "task_log": str(cluster.task_paths[0]),
            "cluster_task_ids": list(cluster.task_ids),
        }

    @staticmethod
    def _next_action(status: str, reason: str, queue_head: dict | None) -> str:
        if status == "previewed":
            return "rerun tidy-agent without --dry-run to apply the current source-cluster step"
        if status == "blocked":
            return "agent performs the manual fix for the current source cluster, then reruns tidy-agent"
        if status == "failed":
            return "inspect the source-cluster automation report and rerun after fixing the failure"
        if reason == "queue_empty_requires_tidy_close":
            return "run tidy-close for the final full tidy and fresh verify gate"
        if queue_head:
            return (
                f"re-resolve the queue and continue from {queue_head['cluster_id']}/"
                f"task_{queue_head['task_id']}"
            )
        return "re-resolve the current queue before continuing"
