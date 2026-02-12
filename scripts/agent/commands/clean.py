from pathlib import Path
from ..core.context import Context

class CleanCommand:
    def __init__(self, ctx: Context):
        self.ctx = ctx

    def execute(self, app_name: str, task_ids: list):
        app_dir = self.ctx.get_app_dir(app_name)
        tasks_dir = app_dir / "build_tidy" / "tasks"
        done_dir = app_dir / "build_tidy" / "tasks_done"
        
        if not tasks_dir.exists():
            return 0
            
        archived_count = 0
        for tid in task_ids:
            padded_id = tid.zfill(3)
            for log_file in tasks_dir.rglob(f"task_{padded_id}.log"):
                relative_path = log_file.relative_to(tasks_dir)
                archive_path = done_dir / relative_path
                archive_path.parent.mkdir(parents=True, exist_ok=True)
                if archive_path.exists():
                    archive_path.unlink()
                log_file.replace(archive_path)
                archived_count += 1

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
        return 0
