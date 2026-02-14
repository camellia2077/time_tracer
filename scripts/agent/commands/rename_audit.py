from pathlib import Path
from typing import Dict


class RenameAuditMixin:
    def _process_audit_candidate(
        self,
        index: int,
        candidate: Dict,
        app_dir: Path,
    ) -> Dict:
        file_path = self._resolve_file_path(candidate.get("file", ""), app_dir)
        old_name = candidate.get("old_name", "")
        new_name = candidate.get("new_name", "")
        if not self._is_within_app_scope(file_path, app_dir):
            status = "out_of_scope"
            old_count = 0
            new_count = 0
        elif not file_path.exists():
            status = "missing_file"
            old_count = 0
            new_count = 0
        else:
            text = file_path.read_text(encoding="utf-8", errors="replace")
            old_count = self._count_symbol_occurrences(text, old_name)
            new_count = self._count_symbol_occurrences(text, new_name)
            status = "resolved" if old_count == 0 and new_count > 0 else "pending"

            symbol_kind = candidate.get("symbol_kind", "")
            if (
                status == "resolved"
                and self._is_header_file(file_path)
                and self._is_risky_symbol_kind(symbol_kind)
            ):
                sibling_old_count, sibling_new_count = self._count_symbol_in_sibling_sources(
                    header_path=file_path,
                    old_name=old_name,
                    new_name=new_name,
                )
                old_count += sibling_old_count
                new_count += sibling_new_count
                if sibling_old_count > 0:
                    status = "pending"

        return {
            "id": index,
            "status": status,
            "file": str(file_path),
            "line": int(candidate.get("line", 0)),
            "col": int(candidate.get("col", 0)),
            "old_name": old_name,
            "new_name": new_name,
            "old_count": old_count,
            "new_count": new_count,
        }

    def audit(self, app_name: str, strict: bool = False) -> int:
        paths = self._paths(app_name)
        candidates = self._load_candidates(paths["candidates_path"])
        if not candidates:
            print("--- No rename candidates found for audit.")
            print("--- Run `python scripts/run.py rename-plan --app <app>` first.")
            return 1

        results = []
        status_counts = {"resolved": 0, "pending": 0, "missing_file": 0, "out_of_scope": 0}

        for index, candidate in enumerate(candidates, 1):
            result = self._process_audit_candidate(index, candidate, paths["app_dir"])
            status_counts[result["status"]] += 1
            results.append(result)

        self._write_audit_report(
            json_path=paths["audit_report_json_path"],
            markdown_path=paths["audit_report_md_path"],
            app_name=app_name,
            status_counts=status_counts,
            results=results,
        )

        print("--- Rename audit complete.")
        print(
            f"--- Resolved: {status_counts['resolved']}, "
            f"Pending: {status_counts['pending']}, "
            f"Missing file: {status_counts['missing_file']}, "
            f"Out of scope: {status_counts['out_of_scope']}"
        )
        print(f"--- Report JSON: {paths['audit_report_json_path']}")
        print(f"--- Report Markdown: {paths['audit_report_md_path']}")

        if strict and (
            status_counts["pending"] > 0
            or status_counts["missing_file"] > 0
            or status_counts["out_of_scope"] > 0
        ):
            return 1
        return 0
