from pathlib import Path

from ...core.context import Context
from ...services import batch_state
from ..shared import tidy as tidy_shared


class CleanCommand:
    def __init__(self, ctx: Context):
        self.ctx = ctx

    def execute(
        self,
        app_name: str,
        task_ids: list[str],
        strict: bool = False,
        batch_id: str | None = None,
    ) -> int:
        app_dir = self.ctx.get_app_dir(app_name)
        tasks_dir = app_dir / "build_tidy" / "tasks"
        done_dir = app_dir / "build_tidy" / "tasks_done"
        try:
            normalized_batch = self._normalize_batch_name(batch_id)
        except ValueError as exc:
            print(f"--- clean: {exc}")
            return 2

        verify_success: bool | None = None
        if strict:
            verify_ok, verify_reason = self._latest_verify_succeeded(app_name=app_name)
            if not verify_ok:
                print(
                    "--- clean: strict mode rejected because latest verify result "
                    f"is not successful ({verify_reason})."
                )
                return 1
            verify_success = True

        if not tasks_dir.exists():
            batch_state_path = batch_state.update_state(
                ctx=self.ctx,
                app_name=app_name,
                batch_id=normalized_batch,
                cleaned_task_ids=[],
                last_verify_success=verify_success,
                extra_fields={"last_clean_ok": True},
            )
            print(f"--- clean: batch state updated -> {batch_state_path}")
            return 0

        archived_count = 0
        cleaned_task_ids: list[str] = []
        for tid in task_ids:
            padded_id = tid.zfill(3)
            target_logs = self._resolve_task_logs(
                tasks_dir=tasks_dir,
                padded_id=padded_id,
                normalized_batch=normalized_batch,
            )
            if normalized_batch and not target_logs:
                print(
                    f"--- clean: task_{padded_id}.log not found in {tasks_dir / normalized_batch}"
                )
                return 1

            for log_file in target_logs:
                relative_path = log_file.relative_to(tasks_dir)
                archive_path = done_dir / relative_path
                archive_path.parent.mkdir(parents=True, exist_ok=True)
                if archive_path.exists():
                    archive_path.unlink()
                log_file.replace(archive_path)
                archived_count += 1
                if padded_id not in cleaned_task_ids:
                    cleaned_task_ids.append(padded_id)

        batch_dirs = [path for path in tasks_dir.glob("batch_*") if path.is_dir()]
        batch_dirs.sort(key=lambda path: path.name, reverse=True)
        removed_batch_dirs = 0
        for batch_dir in batch_dirs:
            if any(batch_dir.iterdir()):
                continue
            batch_dir.rmdir()
            removed_batch_dirs += 1

        print(
            f"--- Archived {archived_count} task logs to {done_dir} "
            f"and removed {removed_batch_dirs} empty batch folders from {tasks_dir}"
        )
        batch_state_path = batch_state.update_state(
            ctx=self.ctx,
            app_name=app_name,
            batch_id=normalized_batch,
            cleaned_task_ids=cleaned_task_ids,
            last_verify_success=verify_success,
            extra_fields={"last_clean_ok": True},
        )
        print(f"--- clean: batch state updated -> {batch_state_path}")
        return 0

    def _resolve_task_logs(
        self,
        tasks_dir: Path,
        padded_id: str,
        normalized_batch: str | None,
    ) -> list[Path]:
        if not normalized_batch:
            return list(tasks_dir.rglob(f"task_{padded_id}.log"))

        batch_dir = tasks_dir / normalized_batch
        if not batch_dir.exists() or not batch_dir.is_dir():
            return []
        return list(batch_dir.glob(f"task_{padded_id}.log"))

    def _normalize_batch_name(self, batch_id: str | None) -> str | None:
        return tidy_shared.normalize_batch_name(batch_id, allow_none=True)

    def _latest_verify_succeeded(self, app_name: str) -> tuple[bool, str]:
        return tidy_shared.latest_verify_succeeded(self.ctx, app_name)
