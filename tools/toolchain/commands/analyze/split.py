from __future__ import annotations

import hashlib
import json
from pathlib import Path

from ..shared import tidy as shared_json
from .issue_model import (
    AnalyzeIssue,
    AnalyzeIssueLocation,
    ISSUE_RECORD_VERSION,
    render_toon,
)
from .sarif import extract_events, extract_primary_location, infer_category, read_result_message

DEFAULT_ANALYZE_BATCH_SIZE = 10


def split_sarif_report(
    *,
    raw_report_path: Path,
    issues_dir: Path,
    summary_path: Path,
    workspace_name: str,
    source_scope: str | None,
    batch_size: int = DEFAULT_ANALYZE_BATCH_SIZE,
) -> dict:
    if batch_size <= 0:
        raise ValueError("batch_size must be > 0")
    payload = json.loads(raw_report_path.read_text(encoding="utf-8"))
    issues = collect_issues(
        payload,
        workspace_name=workspace_name,
        source_scope=source_scope,
        raw_report_path=raw_report_path,
        batch_size=batch_size,
    )
    cleanup_old_issues(issues_dir)
    total_batches = write_issue_batches(issues, issues_dir, batch_size=batch_size)
    summary = refresh_summary(
        summary_path,
        raw_report_path=raw_report_path,
        issues_dir=issues_dir,
        issue_count=len(issues),
        batch_count=total_batches,
        workspace_name=workspace_name,
        source_scope=source_scope,
    )
    return {
        "issues": len(issues),
        "batches": total_batches,
        "batch_size": batch_size,
        "summary": summary,
    }


def collect_issues(
    payload: dict,
    *,
    workspace_name: str,
    source_scope: str | None,
    raw_report_path: Path,
    batch_size: int,
) -> list[AnalyzeIssue]:
    drafts: list[dict] = []
    for run_index, run in enumerate(payload.get("runs", []), start=1):
        if not isinstance(run, dict):
            continue
        for result_index, result in enumerate(run.get("results", []), start=1):
            if not isinstance(result, dict):
                continue
            drafts.append(
                build_issue_draft(
                    run=run,
                    result=result,
                    run_index=run_index,
                    result_index=result_index,
                    workspace_name=workspace_name,
                    source_scope=source_scope,
                    raw_report_path=raw_report_path,
                )
            )

    drafts.sort(
        key=lambda item: (
            item["source_file"],
            item["primary_location"]["line"],
            item["primary_location"]["col"],
            item["rule_id"],
            item["fingerprint"],
        )
    )

    issues: list[AnalyzeIssue] = []
    for issue_index, draft in enumerate(drafts, start=1):
        batch_id = f"batch_{((issue_index - 1) // batch_size) + 1:03d}"
        issues.append(
            finalize_issue(
                draft,
                issue_id=f"{issue_index:03d}",
                batch_id=batch_id,
            )
        )
    return issues


def build_issue_draft(
    *,
    run: dict,
    result: dict,
    run_index: int,
    result_index: int,
    workspace_name: str,
    source_scope: str | None,
    raw_report_path: Path,
) -> dict:
    rule_id = str(result.get("ruleId") or "unknown").strip() or "unknown"
    severity = str(result.get("level") or "warning").strip() or "warning"
    message = read_result_message(result)
    primary_location = extract_primary_location(run, result)
    events = extract_events(run, result)
    source_file = str(primary_location.get("file") or "").strip()
    if not source_file and events:
        source_file = str(events[0].get("file") or "").strip()
    category = infer_category(result, rule_id)
    fingerprint = build_issue_fingerprint(
        rule_id=rule_id,
        message=message,
        source_file=source_file,
        primary_location=primary_location,
    )
    return {
        "workspace": workspace_name,
        "source_scope": source_scope,
        "source_file": source_file,
        "rule_id": rule_id,
        "category": category,
        "severity": severity,
        "message": message,
        "primary_location": primary_location,
        "events": events,
        "fingerprint": fingerprint,
        "raw_report": {
            "path": str(raw_report_path),
            "run_index": run_index,
            "result_index": result_index,
        },
    }


def finalize_issue(draft: dict, *, issue_id: str, batch_id: str) -> AnalyzeIssue:
    primary_location = AnalyzeIssueLocation(
        file=str(draft["primary_location"].get("file") or ""),
        line=int(draft["primary_location"].get("line") or 0),
        col=int(draft["primary_location"].get("col") or 0),
        end_line=int(draft["primary_location"].get("end_line") or 0),
        end_col=int(draft["primary_location"].get("end_col") or 0),
        message=str(draft["primary_location"].get("message") or ""),
    )
    events = tuple(
        AnalyzeIssueLocation(
            file=str(item.get("file") or ""),
            line=int(item.get("line") or 0),
            col=int(item.get("col") or 0),
            end_line=int(item.get("end_line") or 0),
            end_col=int(item.get("end_col") or 0),
            message=str(item.get("message") or ""),
        )
        for item in draft["events"]
    )
    return AnalyzeIssue(
        version=ISSUE_RECORD_VERSION,
        issue_id=issue_id,
        batch_id=batch_id,
        workspace=str(draft["workspace"]),
        source_scope=draft["source_scope"],
        source_file=str(draft["source_file"]),
        rule_id=str(draft["rule_id"]),
        category=str(draft["category"]),
        severity=str(draft["severity"]),
        message=str(draft["message"]),
        primary_location=primary_location,
        events=events,
        fingerprint=str(draft["fingerprint"]),
        raw_report=dict(draft["raw_report"]),
    )


def build_issue_fingerprint(
    *,
    rule_id: str,
    message: str,
    source_file: str,
    primary_location: dict,
) -> str:
    raw = "|".join(
        [
            rule_id,
            message,
            source_file,
            str(primary_location.get("line") or 0),
            str(primary_location.get("col") or 0),
        ]
    )
    return hashlib.sha1(raw.encode("utf-8")).hexdigest()


def write_issue_batches(
    issues: list[AnalyzeIssue],
    issues_dir: Path,
    *,
    batch_size: int,
) -> int:
    issues_dir.mkdir(parents=True, exist_ok=True)
    for issue_index, issue in enumerate(issues, start=1):
        batch_index = ((issue_index - 1) // batch_size) + 1
        batch_name = f"batch_{batch_index:03d}"
        batch_dir = issues_dir / batch_name
        batch_dir.mkdir(parents=True, exist_ok=True)
        issue_base_path = batch_dir / f"issue_{issue.issue_id}"
        shared_json.write_json_dict(issue_base_path.with_suffix(".json"), issue.to_dict())
        issue_base_path.with_suffix(".toon").write_text(
            render_toon(issue),
            encoding="utf-8",
        )
    if not issues:
        return 0
    return ((len(issues) - 1) // batch_size) + 1


def cleanup_old_issues(issues_dir: Path) -> None:
    if not issues_dir.exists():
        return
    for old_issue in issues_dir.rglob("issue_*.*"):
        old_issue.unlink()
    batch_dirs = [path for path in issues_dir.glob("batch_*") if path.is_dir()]
    batch_dirs.sort(key=lambda path: path.name, reverse=True)
    for batch_dir in batch_dirs:
        if any(batch_dir.iterdir()):
            continue
        batch_dir.rmdir()


def refresh_summary(
    summary_path: Path,
    *,
    raw_report_path: Path,
    issues_dir: Path,
    issue_count: int,
    batch_count: int,
    workspace_name: str,
    source_scope: str | None,
) -> dict:
    payload = shared_json.read_json_dict(summary_path) or {
        "version": 1,
        "workspace": workspace_name,
        "source_scope": source_scope,
        "raw_report": str(raw_report_path),
    }
    payload["split"] = {
        "issues_dir": str(issues_dir),
        "issue_count": issue_count,
        "batch_count": batch_count,
    }
    shared_json.write_json_dict(summary_path, payload)
    return payload
