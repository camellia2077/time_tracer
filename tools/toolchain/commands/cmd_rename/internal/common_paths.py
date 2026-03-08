import json
from pathlib import Path

from ...tidy.workspace import DEFAULT_TIDY_BUILD_DIR_NAME


def paths(ctx, app_name: str, tidy_build_dir_name: str | None = None) -> dict[str, Path]:
    app_dir = ctx.get_app_dir(app_name)
    resolved_tidy_build_dir_name = (
        (tidy_build_dir_name or "").strip() or DEFAULT_TIDY_BUILD_DIR_NAME
    )
    tidy_layout = ctx.get_tidy_layout(app_name, resolved_tidy_build_dir_name)
    build_tidy_dir = tidy_layout.root
    rename_dir = tidy_layout.rename_dir
    return {
        "app_dir": app_dir,
        "build_tidy_dir": build_tidy_dir,
        "tasks_dir": tidy_layout.tasks_dir,
        "rename_dir": rename_dir,
        "candidates_path": rename_dir / "rename_candidates.json",
        "plan_md_path": rename_dir / "rename_plan.md",
        "apply_report_json_path": rename_dir / "rename_apply_report.json",
        "apply_report_md_path": rename_dir / "rename_apply_report.md",
        "audit_report_json_path": rename_dir / "rename_audit_report.json",
        "audit_report_md_path": rename_dir / "rename_audit_report.md",
    }


def load_candidates(candidates_path: Path) -> list[dict]:
    if not candidates_path.exists():
        return []
    data = json.loads(candidates_path.read_text(encoding="utf-8"))
    return data.get("candidates", [])


def is_within_app_scope(file_path: Path, app_dir: Path) -> bool:
    try:
        file_path.resolve().relative_to(app_dir.resolve())
        return True
    except Exception:
        return False


def resolve_file_path(file_path: str, app_dir: Path) -> Path:
    candidate_path = Path(file_path)
    if candidate_path.is_absolute():
        return candidate_path
    return app_dir / candidate_path
