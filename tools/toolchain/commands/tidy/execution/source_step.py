from __future__ import annotations

from dataclasses import replace

from ....core.context import Context
from ...cmd_build import BuildCommand
from .clean import CleanCommand
from .refresh_internal import refresh_runner as tidy_refresh_runner
from .step_internal.recheck import TaskRecheckResult, run_task_recheck
from .step_internal.source_step_artifacts import refresh_cluster_artifacts
from .step_internal.source_step_state import write_source_step_state
from ..queue.source_cluster import (
    SourceTaskCluster,
    resolve_task_source_cluster,
)
from ..queue.task_auto_fix import run_task_auto_fix
from ..queue.task_context import resolve_task_context
from ..queue.task_fingerprint import compute_source_fingerprint, fingerprints_match
from ..workspace import resolve_workspace


class TidySourceStepCommand:
    """Process one current source-file cluster as one recheck unit."""

    def __init__(self, ctx: Context):
        self.ctx = ctx

    def execute(
        self,
        *,
        task_log_path: str,
        verify_build_dir_name: str | None = None,
        profile_name: str | None = None,
        concise: bool = False,
        kill_build_procs: bool = False,
        dry_run: bool = False,
        strict: bool = False,
        config_file: str | None = None,
        strict_config: bool = False,
    ) -> int:
        task_ctx = resolve_task_context(self.ctx, task_log_path=task_log_path)
        workspace = resolve_workspace(
            self.ctx,
            build_dir_name=task_ctx.tidy_build_dir_name,
            source_scope=task_ctx.source_scope,
        )
        cluster = resolve_task_source_cluster(
            task_ctx.tasks_dir,
            task_ctx.task_json_path,
        )
        if not cluster.task_paths:
            raise FileNotFoundError("source cluster has no pending task artifacts")

        print(
            f"--- tidy-source-step: source={cluster.source_file}, "
            f"cluster={cluster.cluster_id}, tasks={','.join(cluster.task_ids)}"
        )

        if self._cluster_is_stale(cluster):
            print(
                "--- tidy-source-step: source fingerprint drift detected; "
                "refreshing the current source cluster before applying fixes."
            )
            if dry_run:
                print("--- tidy-source-step: dry-run mode, stale cluster requires a real re-check.")
                return 2
            build_ret = self._build(
                app_name=task_ctx.app_name,
                build_dir_name=verify_build_dir_name,
                profile_name=profile_name,
                concise=concise,
                kill_build_procs=kill_build_procs,
            )
            if build_ret != 0:
                return build_ret
            stale_recheck = self._run_cluster_recheck(
                task_ctx=task_ctx,
                cluster=cluster,
                workspace=workspace,
                config_file=config_file,
                strict_config=strict_config,
            )
            if stale_recheck.exit_code != 0:
                return stale_recheck.exit_code
            if not stale_recheck.diagnostics:
                return self._archive_cluster(
                    task_ctx=task_ctx,
                    cluster=cluster,
                    recheck=stale_recheck,
                    workspace=workspace,
                    config_file=config_file,
                    strict_config=strict_config,
                )
            self._refresh_cluster_artifacts(
                task_ctx=task_ctx,
                cluster=cluster,
                diagnostics=list(stale_recheck.diagnostics),
            )
            self._write_state(
                task_ctx=task_ctx,
                cluster=cluster,
                status="stale_refreshed",
                exit_code=2,
                recheck=stale_recheck,
                workspace=workspace,
                config_file=config_file,
                strict_config=strict_config,
                next_action="re-resolve the current source cluster before editing",
            )
            return 2

        fix_results = []
        for task_path in cluster.task_paths:
            result = run_task_auto_fix(
                self.ctx,
                task_log_path=str(task_path),
                dry_run=dry_run,
                report_suffix="source_step",
            )
            fix_results.append(result)
            print(
                f"--- tidy-task-fix: task={result.task_id}, applied={result.applied}, "
                f"previewed={result.previewed}, skipped={result.skipped}, failed={result.failed}"
            )
            if result.failed and not self._can_continue_after_fix_failures(result):
                return result.exit_code(strict=strict)

        if dry_run:
            print("--- tidy-source-step: dry-run complete; source files unchanged.")
            return 0

        build_ret = self._build(
            app_name=task_ctx.app_name,
            build_dir_name=verify_build_dir_name,
            profile_name=profile_name,
            concise=concise,
            kill_build_procs=kill_build_procs,
        )
        if build_ret != 0:
            return build_ret

        recheck = self._run_cluster_recheck(
            task_ctx=task_ctx,
            cluster=cluster,
            workspace=workspace,
            config_file=config_file,
            strict_config=strict_config,
        )
        if recheck.exit_code != 0:
            self._write_state(
                task_ctx=task_ctx,
                cluster=cluster,
                status="failed",
                exit_code=recheck.exit_code,
                recheck=recheck,
                workspace=workspace,
                config_file=config_file,
                strict_config=strict_config,
                next_action="inspect the focused re-check log",
            )
            return recheck.exit_code
        if recheck.diagnostics:
            self._refresh_cluster_artifacts(
                task_ctx=task_ctx,
                cluster=cluster,
                diagnostics=list(recheck.diagnostics),
            )
            self._write_state(
                task_ctx=task_ctx,
                cluster=cluster,
                status="manual",
                exit_code=2,
                recheck=recheck,
                workspace=workspace,
                config_file=config_file,
                strict_config=strict_config,
                next_action="agent applies the smallest justified source fix, then re-resolves and reruns the cluster",
            )
            return 2

        return self._archive_cluster(
            task_ctx=task_ctx,
            cluster=cluster,
            recheck=recheck,
            workspace=workspace,
            config_file=config_file,
            strict_config=strict_config,
        )

    def _cluster_is_stale(self, cluster: SourceTaskCluster) -> bool:
        current = compute_source_fingerprint(cluster.source_file)
        return any(
            not fingerprints_match(record.source_fingerprint, current)
            for record in cluster.task_records
        )

    def _run_cluster_recheck(
        self,
        *,
        task_ctx,
        cluster: SourceTaskCluster,
        workspace,
        config_file: str | None,
        strict_config: bool,
    ) -> TaskRecheckResult:
        first = cluster.task_records[0]
        combined = replace(
            first,
            checks=tuple(dict.fromkeys(check for record in cluster.task_records for check in record.checks)),
            diagnostics=tuple(
                diagnostic
                for record in cluster.task_records
                for diagnostic in record.diagnostics
            ),
        )
        print("--- tidy-source-step: running focused clang-tidy re-check...")
        return run_task_recheck(
            self.ctx,
            app_name=task_ctx.app_name,
            parsed=combined,
            tidy_build_dir_name=workspace.build_dir_name,
            source_scope=workspace.source_scope,
            config_file=config_file,
            strict_config=strict_config,
            refresh_runner=tidy_refresh_runner,
        )

    def _archive_cluster(
        self,
        *,
        task_ctx,
        cluster,
        recheck,
        workspace,
        config_file: str | None,
        strict_config: bool,
    ) -> int:
        print(
            f"--- tidy-source-step: re-check passed; archiving source cluster "
            f"({','.join(cluster.task_ids)})..."
        )
        clean_ret = CleanCommand(self.ctx).execute(
            app_name=task_ctx.app_name,
            task_ids=list(cluster.task_ids),
            cluster_id=cluster.cluster_id,
            tidy_build_dir_name=workspace.build_dir_name,
        )
        if clean_ret != 0:
            return clean_ret
        self._write_state(
            task_ctx=task_ctx,
            cluster=cluster,
            status="resolved",
            exit_code=0,
            recheck=recheck,
            workspace=workspace,
            config_file=config_file,
            strict_config=strict_config,
            next_action="re-resolve the current queue head",
        )
        return 0

    def _refresh_cluster_artifacts(self, *, task_ctx, cluster, diagnostics: list[dict]) -> None:
        refresh_cluster_artifacts(
            task_ctx=task_ctx,
            cluster=cluster,
            diagnostics=diagnostics,
        )

    def _write_state(
        self,
        *,
        task_ctx,
        cluster,
        status: str,
        exit_code: int,
        recheck: TaskRecheckResult,
        workspace,
        config_file: str | None,
        strict_config: bool,
        next_action: str,
    ) -> None:
        write_source_step_state(
            ctx=self.ctx,
            task_ctx=task_ctx,
            cluster=cluster,
            status=status,
            exit_code=exit_code,
            recheck=recheck,
            workspace=workspace,
            config_file=config_file,
            strict_config=strict_config,
            next_action=next_action,
        )

    def _build(self, **kwargs) -> int:
        return BuildCommand(self.ctx).build(
            app_name=kwargs["app_name"],
            tidy=False,
            build_dir_name=kwargs["build_dir_name"],
            profile_name=kwargs["profile_name"],
            concise=kwargs["concise"],
            kill_build_procs=kwargs["kill_build_procs"],
        )

    @staticmethod
    def _can_continue_after_fix_failures(result) -> bool:
        from .step_internal.stale_recovery import can_continue_after_fix_failures

        return can_continue_after_fix_failures(result)
