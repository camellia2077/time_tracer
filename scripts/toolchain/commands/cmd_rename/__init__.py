from ...core.context import Context
from ...services import rename_planner
from ..tidy import TidyCommand
from .apply import RenameApplyMixin
from .audit import RenameAuditMixin
from .common import RenameCommonMixin
from .report import RenameReportMixin


class RenameCommand(
    RenameApplyMixin,
    RenameAuditMixin,
    RenameReportMixin,
    RenameCommonMixin,
):
    def __init__(self, ctx: Context):
        self.ctx = ctx

    def plan(
        self,
        app_name: str,
        max_candidates: int | None = None,
        run_tidy: bool = False,
    ) -> int:
        paths = self._paths(app_name)

        if run_tidy:
            print("--- Running tidy before rename plan generation...")
            ret = TidyCommand(self.ctx).execute(app_name, [])
            if ret != 0:
                return ret

        tasks_dir = paths["tasks_dir"]
        if not tasks_dir.exists():
            print(f"--- Tasks directory does not exist: {tasks_dir}")
            print("--- Run `python scripts/run.py tidy --app <app>` first.")
            return 1

        check_name = self.ctx.config.rename.check_name
        all_candidates = rename_planner.collect_rename_candidates(
            tasks_dir=tasks_dir,
            check_name=check_name,
            allowed_kinds=self.ctx.config.rename.allowed_kinds,
            app_root=paths["app_dir"],
        )
        effective_limit = (
            max_candidates
            if max_candidates is not None
            else self.ctx.config.rename.max_candidates_per_run
        )
        outputs = rename_planner.write_plan_outputs(
            rename_dir=paths["rename_dir"],
            all_candidates=all_candidates,
            check_name=check_name,
            max_candidates=effective_limit,
        )

        print(
            "--- Rename plan generated: "
            f"{len(outputs['selected_candidates'])}/{outputs['total_candidates']} candidates"
        )
        print(f"--- JSON: {outputs['json_path']}")
        print(f"--- Markdown: {outputs['md_path']}")
        if outputs["truncated"]:
            print("--- Plan is truncated by max candidate limit.")
        return 0


__all__ = ["RenameCommand"]
