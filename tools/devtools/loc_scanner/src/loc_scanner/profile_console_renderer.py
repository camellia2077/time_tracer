from pathlib import Path

from .config import ScanProfile


def print_profile_scan_report(
    *,
    profile: ScanProfile,
    mode: str,
    groups: list[dict],
    priority_groups: list[dict],
    missing_paths: list[Path],
    report_paths: dict[str, str],
    module_summaries: list[dict] | None = None,
    module_reading_candidates: list[dict] | None = None,
) -> None:
    print("=" * 100)
    mode_text = "large-file" if mode == "over" else "small-file"
    print(f"{profile.display_name} profile scan report ({mode_text} mode)")
    print("=" * 100)
    print()

    print("[MODULE READING CANDIDATES - SUGGESTED]")
    if not module_reading_candidates:
        print("  No module reading candidates were generated.")
    else:
        for candidate in module_reading_candidates:
            print(
                f"  #{candidate['reading_rank']} {candidate['display_name']} | "
                f"reading_score {candidate['reading_score']:.2f} | "
                f"P{candidate['priority']}"
            )
            for reason in candidate["reasons"]:
                print(f"    - {reason}")
    print()

    print("[MODULE BASELINE]")
    if not module_summaries:
        print("  No module baseline summary was generated.")
    else:
        for module in module_summaries:
            production = module["source_sets"]["production"]
            tests = module["source_sets"]["tests"]
            inline_tests = module.get("test_evidence", {}).get("inline_tests", {})
            print(
                f"  {module['display_name']} | "
                f"{module['files']} files / {module['lines']} lines | "
                f"PRODUCTION {production['files']} files / {production['lines']} lines | "
                f"PATH TESTS {tests['files']} files / {tests['lines']} lines | "
                f"INLINE TESTS {inline_tests.get('files', 0)} files / "
                f"{inline_tests.get('lines', 0)} estimated lines | "
                f"Hotspots {module['matched_files']} | "
                f"Directories {module['source_directories']}"
            )
            if module.get("labels"):
                print(f"    Labels: {', '.join(module['labels'])}")
                for label in module["labels"]:
                    print(
                        f"      {label}: "
                        f"{module.get('label_reasons', {}).get(label, '')}"
                    )
            for item in module["top_files"][:3]:
                print(
                    f"    {item['lines']:<6} lines | "
                    f"[{item['category'].upper()}/{item['language']}] "
                    f"{item['path']}"
                )
            for signal in module.get("assessment", {}).get("signals", []):
                print(f"    Assessment: {signal['code']} — {signal['reason']}")
        print()

    category_names = {
        "libs": "LIBS",
        "presentation": "PRESENTATION",
        "tests": "TESTS",
    }
    for priority_group in priority_groups:
        priority = priority_group["priority"]
        print(f"[P{priority}]")
        for finding in priority_group["findings"]:
            category = category_names.get(
                finding["category"], finding["category"].upper()
            )
            print(
                f"  {finding['lines']:<6} lines | "
                f"[{category}/{finding['display_name']}/{finding['language']}] "
                f"File \"{finding['path']}\"\n"
                f"    Guidance: {finding.get('guidance', {}).get('summary', '')}"
            )
        print()

    if not priority_groups:
        print("[OK] No files matched the selected threshold.")
        print()

    for path in missing_paths:
        print(f"[ERROR] Component path does not exist; scan skipped: {path}")

    if not groups and not missing_paths:
        print("[OK] No matching files found.")
    print(f"Markdown report: {report_paths['markdown']}")
    print(f"JSON report: {report_paths['json']}")
    if "comparison_markdown" in report_paths:
        print(f"Delta Markdown report: {report_paths['comparison_markdown']}")
        print(f"Delta JSON report: {report_paths['comparison_json']}")
    if "baseline_json" in report_paths:
        print(f"Baseline JSON: {report_paths['baseline_json']}")
    print()
    print("-" * 100)
    print()
