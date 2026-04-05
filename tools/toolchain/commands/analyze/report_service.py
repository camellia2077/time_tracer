from __future__ import annotations

import json
from collections import Counter
from pathlib import Path

from .sarif import extract_primary_location


def merge_sarif_reports(report_paths: list[Path]) -> dict:
    merged = {
        "$schema": (
            "https://docs.oasis-open.org/sarif/sarif/v2.1.0/cos02/"
            "schemas/sarif-schema-2.1.0.json"
        ),
        "runs": [],
    }
    for report_path in report_paths:
        payload = json.loads(report_path.read_text(encoding="utf-8"))
        runs = payload.get("runs")
        if isinstance(runs, list):
            merged["runs"].extend(runs)
    return merged


def build_summary(
    *,
    workspace_name: str,
    source_scope: str | None,
    build_dir: Path,
    raw_report_path: Path,
    log_path: Path,
    matched_units: int,
    analyzed_units: int,
    failed_units: list[dict[str, object]],
    merged_sarif: dict,
) -> dict:
    rule_counts: Counter[str] = Counter()
    file_counts: Counter[str] = Counter()
    result_count = 0
    for run in merged_sarif.get("runs", []):
        if not isinstance(run, dict):
            continue
        for result in run.get("results", []):
            if not isinstance(result, dict):
                continue
            result_count += 1
            rule_id = str(result.get("ruleId") or "unknown").strip() or "unknown"
            rule_counts[rule_id] += 1
            primary_location = extract_primary_location(run, result)
            source_path = str(primary_location.get("file") or "").strip()
            if source_path:
                file_counts[source_path] += 1
    return {
        "version": 1,
        "workspace": workspace_name,
        "source_scope": source_scope,
        "build_dir": str(build_dir),
        "raw_report": str(raw_report_path),
        "log": str(log_path),
        "totals": {
            "matched_units": matched_units,
            "analyzed_units": analyzed_units,
            "failed_units": len(failed_units),
            "results": result_count,
            "files_with_findings": len(file_counts),
        },
        "top_rules": [
            {"rule_id": rule_id, "count": count}
            for rule_id, count in rule_counts.most_common(10)
        ],
        "top_files": [
            {"file": file_path, "count": count}
            for file_path, count in file_counts.most_common(10)
        ],
        "failures": failed_units[:20],
    }
