from pathlib import Path
import shutil

from ....services import tidy_state
from ...shared import tidy as tidy_shared
from ..queue.task_log import list_task_paths, load_task_record
from ..workspace import DEFAULT_TIDY_BUILD_DIR_NAME


class CleanCommand:
    """Archive one fully verified source cluster."""

    def __init__(self, ctx):
        self.ctx = ctx

    def execute(
        self,
        app_name: str,
        task_ids: list[str],
        cluster_id: str,
        strict: bool = False,
        tidy_build_dir_name: str | None = None,
    ) -> int:
        resolved_build_dir = (tidy_build_dir_name or "").strip() or DEFAULT_TIDY_BUILD_DIR_NAME
        layout = self.ctx.get_tidy_layout(app_name, resolved_build_dir)
        cluster_dir = layout.tasks_dir / "clusters" / cluster_id
        if not cluster_dir.is_dir():
            print(f"--- clean: source cluster not found: {cluster_dir}")
            return 1

        records = list_task_paths(layout.tasks_dir, cluster_id=cluster_id)
        if not records:
            print(f"--- clean: source cluster has no task records: {cluster_dir}")
            return 1
        expected_ids = {str(value).zfill(3) for value in task_ids}
        actual_ids = {path.stem.removeprefix("task_") for path in records}
        if expected_ids and not expected_ids.issubset(actual_ids):
            print(
                "--- clean: source cluster task set changed; "
                "re-resolve the current cluster before archiving."
            )
            return 2

        if strict:
            verify_ok, reason = tidy_shared.latest_verify_succeeded(self.ctx, app_name)
            if not verify_ok:
                print(f"--- clean: strict mode rejected ({reason}).")
                return 1

        archived_dir = layout.archive_dir / cluster_id
        if archived_dir.exists():
            shutil.rmtree(archived_dir)
        archived_dir.parent.mkdir(parents=True, exist_ok=True)
        cluster_dir.replace(archived_dir)

        state_path = tidy_state.update_state(
            ctx=self.ctx,
            app_name=app_name,
            tidy_build_dir_name=resolved_build_dir,
            cluster_id=cluster_id,
            cleaned_task_ids=sorted(actual_ids),
            extra_fields={
                "last_cluster_archive_ok": True,
                "queue_requires_reresolve": True,
            },
        )
        print(f"--- Archived source cluster {cluster_id} to {archived_dir}")
        print(f"--- tidy state updated -> {state_path}")
        return 0
