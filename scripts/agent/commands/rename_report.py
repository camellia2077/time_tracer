import json
from pathlib import Path
from typing import Dict, List


class RenameReportMixin:
    def _write_apply_report(
        self,
        json_path: Path,
        markdown_path: Path,
        app_name: str,
        dry_run: bool,
        status_counts: Dict[str, int],
        results: List[Dict],
    ):
        json_path.parent.mkdir(parents=True, exist_ok=True)
        payload = {
            "app": app_name,
            "dry_run": dry_run,
            "summary": status_counts,
            "results": results,
        }
        json_path.write_text(
            json.dumps(payload, ensure_ascii=False, indent=2), encoding="utf-8"
        )

        lines = [
            "# Rename Apply Report",
            "",
            f"- App: `{app_name}`",
            f"- Dry run: `{str(dry_run).lower()}`",
            f"- Applied: `{status_counts['applied']}`",
            f"- Skipped: `{status_counts['skipped']}`",
            f"- Failed: `{status_counts['failed']}`",
            "",
            "| ID | Status | File | Line:Col | Old -> New | Edits | Reason |",
            "| --- | --- | --- | --- | --- | --- | --- |",
        ]

        for item in results:
            lines.append(
                "| {id:03d} | {status} | `{file}` | {line}:{col} | `{old}` -> `{new}` | {edits} | {reason} |".format(
                    id=item["id"],
                    status=item["status"],
                    file=item["file"],
                    line=item["line"],
                    col=item["col"],
                    old=item["old_name"],
                    new=item["new_name"],
                    edits=item["edit_count"],
                    reason=item["reason"],
                )
            )

        markdown_path.write_text("\n".join(lines), encoding="utf-8")

    def _write_audit_report(
        self,
        json_path: Path,
        markdown_path: Path,
        app_name: str,
        status_counts: Dict[str, int],
        results: List[Dict],
    ):
        json_path.parent.mkdir(parents=True, exist_ok=True)
        payload = {
            "app": app_name,
            "summary": status_counts,
            "results": results,
        }
        json_path.write_text(
            json.dumps(payload, ensure_ascii=False, indent=2), encoding="utf-8"
        )

        lines = [
            "# Rename Audit Report",
            "",
            f"- App: `{app_name}`",
            f"- Resolved: `{status_counts['resolved']}`",
            f"- Pending: `{status_counts['pending']}`",
            f"- Missing file: `{status_counts['missing_file']}`",
            f"- Out of scope: `{status_counts.get('out_of_scope', 0)}`",
            "",
            "| ID | Status | File | Line:Col | Old -> New | old_count | new_count |",
            "| --- | --- | --- | --- | --- | --- | --- |",
        ]
        for item in results:
            lines.append(
                "| {id:03d} | {status} | `{file}` | {line}:{col} | `{old}` -> `{new}` | {old_count} | {new_count} |".format(
                    id=item["id"],
                    status=item["status"],
                    file=item["file"],
                    line=item["line"],
                    col=item["col"],
                    old=item["old_name"],
                    new=item["new_name"],
                    old_count=item["old_count"],
                    new_count=item["new_count"],
                )
            )
        markdown_path.write_text("\n".join(lines), encoding="utf-8")
