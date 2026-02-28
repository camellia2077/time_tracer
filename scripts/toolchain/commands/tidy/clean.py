from pathlib import Path

from ...core.context import Context
from ...services import batch_state
from ...services.suite_registry import resolve_suite_name
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
        cluster_by_file: bool = False,
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
        verify_result_mtime: float | None = None
        if strict:
            verify_ok, verify_reason = self._latest_verify_succeeded(app_name=app_name)
            if not verify_ok:
                print(
                    "--- clean: strict mode rejected because latest verify result "
                    f"is not successful ({verify_reason})."
                )
                return 1
            verify_success = True
            verify_result_mtime = self._latest_verify_result_mtime(app_name=app_name)
            if verify_result_mtime is None:
                print(
                    "--- clean: strict mode rejected because verify result timestamp "
                    "is unavailable."
                )
                return 1

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

        effective_task_ids = task_ids
        if cluster_by_file:
            if not normalized_batch:
                print("--- clean: --cluster-by-file requires --batch-id.")
                return 2
            expanded_ids = self._expand_task_ids_by_file(
                tasks_dir=tasks_dir,
                normalized_batch=normalized_batch,
                task_ids=task_ids,
            )
            if not expanded_ids:
                print("--- clean: --cluster-by-file found no eligible tasks to clean.")
                return 1
            if expanded_ids != task_ids:
                print("--- clean: cluster-by-file expanded task ids -> " + ", ".join(expanded_ids))
            effective_task_ids = expanded_ids

        archived_count = 0
        cleaned_task_ids: list[str] = []
        for tid in effective_task_ids:
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
                if strict:
                    strict_ok, strict_reason = self._strict_task_guard(
                        log_file=log_file,
                        verify_result_mtime=verify_result_mtime,
                    )
                    if not strict_ok:
                        print(f"--- clean: strict guard rejected {log_file.name}: {strict_reason}")
                        return 1
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

    def _latest_verify_result_mtime(self, app_name: str) -> float | None:
        suite_name = resolve_suite_name(app_name) or app_name
        result_path = self.ctx.repo_root / "test" / "output" / suite_name / "result.json"
        if not result_path.exists():
            return None
        try:
            return result_path.stat().st_mtime
        except OSError:
            return None

    def _strict_task_guard(
        self,
        *,
        log_file: Path,
        verify_result_mtime: float | None,
    ) -> tuple[bool, str]:
        if verify_result_mtime is None:
            return False, "missing verify result timestamp"
        try:
            task_mtime = log_file.stat().st_mtime
        except OSError:
            return False, "cannot read task timestamp"
        if verify_result_mtime < task_mtime:
            return False, "verify result is older than task log"

        for source_path in self._extract_task_source_files(log_file):
            if not source_path.exists():
                continue
            try:
                source_mtime = source_path.stat().st_mtime
            except OSError:
                continue
            if verify_result_mtime < source_mtime:
                return (
                    False,
                    f"verify result is older than source update ({source_path})",
                )
        return True, "ok"

    def _extract_task_source_files(self, log_file: Path) -> list[Path]:
        files: list[Path] = []
        try:
            lines = log_file.read_text(encoding="utf-8", errors="ignore").splitlines()
        except OSError:
            return files
        for line in lines:
            if not line.startswith("File: "):
                continue
            raw = line[len("File: ") :].strip()
            if not raw:
                continue
            path = Path(raw)
            if path not in files:
                files.append(path)
        return files

    def _expand_task_ids_by_file(
        self,
        *,
        tasks_dir: Path,
        normalized_batch: str,
        task_ids: list[str],
    ) -> list[str]:
        batch_dir = tasks_dir / normalized_batch
        if not batch_dir.exists():
            return []
        logs = sorted(batch_dir.glob("task_*.log"), key=lambda p: p.name)
        if not logs:
            return []

        file_to_ids: dict[str, list[str]] = {}
        id_to_file: dict[str, str] = {}
        for log_file in logs:
            tid = log_file.stem.replace("task_", "").zfill(3)
            file_keys = self._extract_task_source_files(log_file)
            if not file_keys:
                continue
            # Most tasks map to one source file. Use the first as cluster key.
            key = str(file_keys[0])
            id_to_file[tid] = key
            file_to_ids.setdefault(key, []).append(tid)

        expanded: list[str] = []
        for raw_tid in task_ids:
            tid = raw_tid.zfill(3)
            key = id_to_file.get(tid)
            if key is None:
                if tid not in expanded:
                    expanded.append(tid)
                continue
            for cluster_tid in file_to_ids.get(key, []):
                if cluster_tid not in expanded:
                    expanded.append(cluster_tid)
        return expanded
