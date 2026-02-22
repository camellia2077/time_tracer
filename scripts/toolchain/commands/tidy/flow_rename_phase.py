import json

from ...core.context import Context
from ..cmd_rename import RenameCommand
from . import flow_stages as tidy_flow_stages, flow_state as tidy_flow_state
from .command import TidyCommand


def _calc_already_renamed_stats(rename_report_path) -> tuple[int, int]:
    if not rename_report_path.exists():
        return 0, 0
    try:
        payload = json.loads(rename_report_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError):
        return 0, 0
    results = payload.get("results", [])
    if not isinstance(results, list):
        return 0, 0

    total = 0
    already_renamed = 0
    for item in results:
        if not isinstance(item, dict):
            continue
        total += 1
        reason = item.get("reason")
        if reason == "already_renamed":
            already_renamed += 1
    return already_renamed, total


def _should_auto_rebuild_from_rename(ctx: Context, build_tidy_dir) -> tuple[bool, str]:
    if not ctx.config.tidy.auto_full_on_high_already_renamed:
        return False, ""

    rename_report_path = build_tidy_dir / "rename" / "rename_apply_report.json"
    already_renamed, total = _calc_already_renamed_stats(rename_report_path)
    if total <= 0:
        return False, ""

    ratio = already_renamed / total
    min_count = max(1, int(ctx.config.tidy.auto_full_already_renamed_min))
    threshold = float(ctx.config.tidy.auto_full_already_renamed_ratio)
    if already_renamed >= min_count and ratio >= threshold:
        summary = f"{already_renamed}/{total} ({ratio:.1%})"
        return True, summary
    return False, ""


def _run_auto_full_tidy(ctx: Context, options) -> int:
    return TidyCommand(ctx).execute(
        app_name=options.app_name,
        extra_args=[],
        jobs=options.jobs,
        parse_workers=options.parse_workers,
        keep_going=options.keep_going,
        build_dir_name="build_tidy",
    )


def run_rename_phase(ctx: Context, options, state: dict, build_tidy_dir) -> int:
    tidy_flow_state.set_phase(state, "rename")
    rename_cmd = RenameCommand(ctx)

    rename_plan_ret = rename_cmd.plan(options.app_name)
    if rename_plan_ret != 0:
        tidy_flow_state.set_step(state, "rename", "failed", rename_plan_ret)
        return rename_plan_ret

    rename_candidates = tidy_flow_stages.count_rename_candidates(build_tidy_dir)
    state["rename_candidates"] = rename_candidates
    if rename_candidates > 0:
        rename_apply_ret = rename_cmd.apply(options.app_name, strict=True)
        auto_rebuild, summary = _should_auto_rebuild_from_rename(ctx, build_tidy_dir)
        if auto_rebuild:
            print(
                "--- tidy-flow: detected high `already_renamed`, auto running full tidy "
                f"rebuild ({summary})."
            )
            full_ret = _run_auto_full_tidy(ctx, options)
            if full_ret != 0:
                tidy_flow_state.set_step(state, "rename", "failed", full_ret)
                return full_ret
            state["rename_auto_full_reason"] = "auto_high_already_renamed"
            state["rename_auto_full_summary"] = summary
            tidy_flow_state.set_step(state, "rename", "done", 0)
            return 0
        if rename_apply_ret != 0:
            tidy_flow_state.set_step(state, "rename", "failed", rename_apply_ret)
            return rename_apply_ret

        rename_audit_ret = rename_cmd.audit(options.app_name, strict=True)
        if rename_audit_ret != 0:
            tidy_flow_state.set_step(state, "rename", "failed", rename_audit_ret)
            return rename_audit_ret
    else:
        print("--- tidy-flow: no rename candidates, skip rename-apply/rename-audit.")

    tidy_flow_state.set_step(state, "rename", "done", 0)
    return 0
