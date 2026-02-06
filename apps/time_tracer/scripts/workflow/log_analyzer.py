import os
import re
import json
from pathlib import Path
from typing import List, Dict

def analyze_tasks(tasks_dir: Path) -> List[Dict]:
    """
    Parse task logs in tasks_dir and return a structured summary.
    """
    if not tasks_dir.exists():
        return []

    summary = []
    file_pattern = re.compile(r"File: (.*)")
    warning_pattern = re.compile(r".*:\d+:\d+: warning: (.*) \[(.*)\]")
    
    log_files = sorted(tasks_dir.glob("task_*.log"))
    
    for log_file in log_files:
        task_id = log_file.stem.split("_")[1]
        try:
            content = log_file.read_text(encoding="utf-8", errors="ignore")
        except Exception:
            continue
            
        file_path = ""
        file_match = file_pattern.search(content)
        if file_match:
            file_path = file_match.group(1).strip()
            
        warnings = []
        for match in warning_pattern.finditer(content):
            msg, w_type = match.groups()
            warning_entry = {"message": msg.strip(), "type": w_type.strip()}
            if warning_entry not in warnings:
                warnings.append(warning_entry)
                
        summary.append({
            "task_id": task_id,
            "log_file": log_file.name,
            "target_file": file_path,
            "warnings": warnings
        })
    return summary

def generate_markdown_summary(summary: List[Dict], output_path: Path, cwd: Path):
    """
    Generate a Markdown table summary from the parsed tasks.
    """
    md_content = "# Clang-Tidy Tasks Summary\n\n"
    md_content += "| ID | File | Warning Types |\n"
    md_content += "| --- | --- | --- |\n"
    
    for item in summary:
        w_types = ", ".join(set(w["type"] for w in item["warnings"]))
        target = item["target_file"]
        try:
            if target:
                rel_path = os.path.relpath(target, start=cwd)
            else:
                rel_path = "N/A"
        except ValueError:
            rel_path = target if target else "N/A"
            
        md_content += f"| {item['task_id']} | {rel_path} | {w_types} |\n"
        
    output_path.write_text(md_content, encoding="utf-8")

def save_json_summary(summary: List[Dict], output_path: Path):
    """
    Save the parsed tasks summary to a JSON file.
    """
    with open(output_path, "w", encoding="utf-8") as f:
        json.dump(summary, f, indent=2)
def analyze_task_batch(tasks_dir: Path, batch_ids: List[str]) -> List[Dict]:
    """
    Parse specific task logs matching the given IDs.
    """
    if not tasks_dir.exists():
        return []

    summary = []
    file_pattern = re.compile(r"File: (.*)")
    warning_pattern = re.compile(r".*:\d+:\d+: warning: (.*) \[(.*)\]")
    
    # Filter logs by ID
    log_files = []
    for tid in batch_ids:
        # Normalize ID to match filename format (e.g. "1" -> "task_001.log")
        # Try exact match first, then zero-padded
        candidates = list(tasks_dir.glob(f"task_*{tid}.log"))
        # Also try "0" + tid etc if user passes short ID
        if not candidates and len(tid) < 3:
             padded = tid.zfill(3)
             candidates = list(tasks_dir.glob(f"task_*{padded}.log"))
        
        log_files.extend(candidates)

    for log_file in log_files:
        task_id = log_file.stem.split("_")[1]
        try:
            content = log_file.read_text(encoding="utf-8", errors="ignore")
        except Exception:
            continue
            
        file_path = ""
        file_match = file_pattern.search(content)
        if file_match:
            file_path = file_match.group(1).strip()
            
        warnings = []
        for match in warning_pattern.finditer(content):
            msg, w_type = match.groups()
            warning_entry = {"message": msg.strip(), "type": w_type.strip()}
            if warning_entry not in warnings:
                warnings.append(warning_entry)
                
        summary.append({
            "task_id": task_id,
            "log_file": log_file.name,
            "target_file": file_path,
            "warnings": warnings
        })
    return summary

def print_batch_summary(summary: List[Dict]):
    """
    Print a deduplicated summary of the batch.
    """
    if not summary:
        print("No tasks found.")
        return

    # 1. Group by (Target File, Warning Type)
    grouped = {}
    for item in summary:
        target = item["target_file"] or "Unknown"
        # Collect all warning types
        w_types_set = set(w["type"] for w in item["warnings"])
        w_types_sig = ", ".join(sorted(w_types_set))
        
        # Details (first unique detail)
        detail_msg = item["warnings"][0]["message"] if item["warnings"] else "No details"
        
        key = (target, w_types_sig, detail_msg)
        if key not in grouped:
            grouped[key] = []
        grouped[key].append(item["task_id"])

    # 2. Print Report
    print(f"\n{'='*60}")
    print(f"BATCH SUMMARY REPORT ({len(summary)} tasks scanned)")
    print(f"{'='*60}\n")
    
    for (target, w_types, detail), tids in grouped.items():
        tids_str = ", ".join(sorted(tids))
        print(f"FILES: {target}")
        print(f"CHECKS: {w_types}")
        print(f"TASKS: [{len(tids)}] {tids_str}")
        print(f"DETAIL: {detail}")
        print("-" * 40)
