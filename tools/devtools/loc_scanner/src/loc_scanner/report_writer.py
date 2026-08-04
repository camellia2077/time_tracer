import json
from pathlib import Path

from .comparison import compare_profile_reports
from .comparison_report_renderer import build_comparison_markdown
from .profile_report_renderer import build_profile_markdown


def write_profile_report(
    *,
    workspace_root: Path,
    profile_name: str,
    profile_display_name: str,
    scan: dict,
    summary: dict,
    groups: list[dict],
    priority_groups: list[dict],
    missing_paths: list[Path],
    module_summaries: list[dict] | None = None,
    module_reading_candidates: list[dict] | None = None,
) -> dict[str, str]:
    report_dir = _report_directory(workspace_root)
    markdown_path = (report_dir / f"profile_{profile_name}.md").resolve()
    json_path = (report_dir / f"profile_{profile_name}.json").resolve()
    normalized_module_summaries = module_summaries or []
    normalized_reading_candidates = module_reading_candidates or []
    report_payload = {
        "report_type": "loc_profile_context",
        "profile": profile_name,
        "profile_display_name": profile_display_name,
        "scan": scan,
        "summary": summary,
        "results": groups,
        "priority_results": priority_groups,
        "module_summaries": normalized_module_summaries,
        "module_reading_candidates": normalized_reading_candidates,
        "missing_paths": [str(path) for path in missing_paths],
        "report_files": {"markdown": str(markdown_path), "json": str(json_path)},
    }
    json_path.write_text(
        json.dumps(report_payload, indent=2, ensure_ascii=False) + "\n",
        encoding="utf-8",
    )
    markdown_path.write_text(
        build_profile_markdown(
            profile_display_name=profile_display_name,
            scan=scan,
            summary=summary,
            groups=groups,
            priority_groups=priority_groups,
            module_summaries=normalized_module_summaries,
            module_reading_candidates=normalized_reading_candidates,
            missing_paths=missing_paths,
            workspace_root=workspace_root,
        ),
        encoding="utf-8",
    )
    _ensure_report_gitignore(report_dir)
    return {"markdown": str(markdown_path), "json": str(json_path)}


def write_profile_comparison_report(
    *,
    workspace_root: Path,
    profile_name: str,
    baseline_path: Path,
    current_path: Path,
) -> dict[str, object]:
    comparison = compare_profile_reports(baseline_path, current_path)
    report_dir = _report_directory(workspace_root)
    markdown_path = (report_dir / f"profile_{profile_name}_delta.md").resolve()
    json_path = (report_dir / f"profile_{profile_name}_delta.json").resolve()
    json_path.write_text(
        json.dumps(comparison, indent=2, ensure_ascii=False) + "\n",
        encoding="utf-8",
    )
    markdown_path.write_text(
        build_comparison_markdown(comparison),
        encoding="utf-8",
    )
    _ensure_report_gitignore(report_dir)
    return {
        "markdown": str(markdown_path),
        "json": str(json_path),
        "summary": comparison["summary"],
    }


def save_profile_baseline(*, current_path: Path, baseline_path: Path) -> str:
    baseline_path.parent.mkdir(parents=True, exist_ok=True)
    baseline_path.write_bytes(current_path.read_bytes())
    return str(baseline_path.resolve())


def _report_directory(workspace_root: Path) -> Path:
    report_dir = workspace_root / "temp" / "loc_scanner" / "reports"
    report_dir.mkdir(parents=True, exist_ok=True)
    return report_dir


def _ensure_report_gitignore(report_dir: Path) -> None:
    gitignore_path = report_dir / ".gitignore"
    if not gitignore_path.exists():
        gitignore_path.write_text(
            "# Automatically created by loc_scanner.\n*\n",
            encoding="utf-8",
        )
