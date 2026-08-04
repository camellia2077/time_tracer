import json
from pathlib import Path
from typing import Any


def compare_profile_reports(
    baseline_path: Path,
    current_path: Path,
) -> dict[str, Any]:
    baseline = _load_profile_report(baseline_path)
    current = _load_profile_report(current_path)
    baseline_findings = _flatten_findings(baseline)
    current_findings = _flatten_findings(current)

    baseline_keys = set(baseline_findings)
    current_keys = set(current_findings)
    added = [current_findings[key] for key in current_keys - baseline_keys]
    removed = [baseline_findings[key] for key in baseline_keys - current_keys]
    changed = []
    for key in baseline_keys & current_keys:
        before = baseline_findings[key]
        after = current_findings[key]
        if before["lines"] != after["lines"]:
            changed.append(
                {
                    "category": after["category"],
                    "component": after["component"],
                    "language": after["language"],
                    "path": after["path"],
                    "before_lines": before["lines"],
                    "after_lines": after["lines"],
                    "delta_lines": after["lines"] - before["lines"],
                }
            )

    added.sort(key=lambda item: (-item["lines"], item["path"].casefold()))
    removed.sort(key=lambda item: (-item["lines"], item["path"].casefold()))
    changed.sort(
        key=lambda item: (-abs(item["delta_lines"]), item["path"].casefold())
    )
    return {
        "report_type": "loc_profile_context_delta",
        "baseline": str(baseline_path.resolve()),
        "current": str(current_path.resolve()),
        "summary": {
            "baseline_matched_files": len(baseline_findings),
            "current_matched_files": len(current_findings),
            "added_files": len(added),
            "removed_files": len(removed),
            "changed_files": len(changed),
            "net_matched_files": len(current_findings) - len(baseline_findings),
        },
        "added": added,
        "removed": removed,
        "changed": changed,
    }


def _load_profile_report(path: Path) -> dict[str, Any]:
    with path.open(encoding="utf-8") as handle:
        payload = json.load(handle)
    if payload.get("report_type") not in {
        "loc_profile_context",
        "loc_profile_refactoring",
    }:
        raise ValueError(f"Not a profile scan report: {path}")
    if not isinstance(payload.get("priority_results"), list):
        raise ValueError(f"Profile scan report is missing priority_results: {path}")
    return payload


def _flatten_findings(report: dict[str, Any]) -> dict[str, dict[str, Any]]:
    findings: dict[str, dict[str, Any]] = {}
    for priority_group in report["priority_results"]:
        for finding in priority_group.get("findings", []):
            key = "|".join(
                (
                    str(finding.get("category", "")),
                    str(finding.get("language", "")),
                    str(finding.get("path", "")),
                )
            )
            findings[key] = {
                "category": finding.get("category", ""),
                "component": finding.get("component", ""),
                "language": finding.get("language", ""),
                "path": finding.get("path", ""),
                "lines": int(finding.get("lines", 0)),
            }
    return findings
