# builder/task_splitter.py
"""
Module to split Clang-Tidy log output into individual task files.

This module parses the ninja tidy output and creates separate task_XXX.log files
for each source file that has warnings, enabling the workflow to process them
individually.

Features:
- Sorts tasks by size (smaller/simpler first)
- Sub-splits large tasks (>50 lines) by header file
- Adds summary header showing files and warning types
"""

import re
from collections import defaultdict
from pathlib import Path
from typing import List, Dict, Optional, Tuple

# Pattern to match task start lines like: [1/146] [14/CHECK] Analyzing: src/file.cpp
TASK_START_PATTERN = re.compile(
    r"^\[(\d+)/(\d+)\]\s+\[\d+/CHECK\]\s+Analyzing:\s+(.+)$"
)

# Pattern to match warning lines with file path
# Example: C:/path/to/file.hpp:42:8: warning: message [check-name]
WARNING_LINE_PATTERN = re.compile(
    r"^([A-Za-z]:)?([^:]+):(\d+):(\d+):\s+warning:\s+(.+)\s+\[([^\]]+)\]"
)

# Threshold for sub-splitting (lines)
LARGE_TASK_THRESHOLD = 50


def _extract_warnings(lines: List[str]) -> List[Dict]:
    """Extract warning details from log lines."""
    warnings = []
    for line in lines:
        match = WARNING_LINE_PATTERN.match(line.strip())
        if match:
            drive = match.group(1) or ""
            file_path = drive + match.group(2)
            warnings.append({
                "file": file_path,
                "line": int(match.group(3)),
                "col": int(match.group(4)),
                "message": match.group(5),
                "check": match.group(6),
            })
    return warnings


def _generate_summary(warnings: List[Dict]) -> str:
    """Generate a summary header for the task."""
    if not warnings:
        return ""
    
    # Count by file
    file_counts: Dict[str, int] = defaultdict(int)
    check_counts: Dict[str, int] = defaultdict(int)
    
    for w in warnings:
        # Extract just the filename from full path
        filename = Path(w["file"]).name
        file_counts[filename] += 1
        check_counts[w["check"]] += 1
    
    lines = ["=== SUMMARY ==="]
    
    # Files (sorted by count descending)
    file_parts = [f"{f}({c})" for f, c in sorted(file_counts.items(), key=lambda x: -x[1])]
    lines.append(f"Files: {', '.join(file_parts)}")
    
    # Warning types (sorted by count descending)
    check_parts = [f"{c}({n})" for c, n in sorted(check_counts.items(), key=lambda x: -x[1])]
    lines.append(f"Types: {', '.join(check_parts)}")
    
    lines.append("=" * 15)
    lines.append("")
    
    return "\n".join(lines)


def _group_by_header(lines: List[str], warnings: List[Dict]) -> Dict[str, Tuple[List[str], List[Dict]]]:
    """
    Group warning lines by the header file they belong to.
    Returns dict: header_filename -> (relevant_lines, relevant_warnings)
    """
    # Build a mapping of which lines belong to which header
    header_groups: Dict[str, List[str]] = defaultdict(list)
    header_warnings: Dict[str, List[Dict]] = defaultdict(list)
    
    # Add the task start line to all groups (first line typically)
    task_start_line = None
    for line in lines:
        if TASK_START_PATTERN.match(line.strip()):
            task_start_line = line
            break
    
    for w in warnings:
        header_name = Path(w["file"]).name
        header_warnings[header_name].append(w)
    
    # For each warning, find the related lines (the warning line + context)
    for line in lines:
        stripped = line.strip()
        if not stripped or TASK_START_PATTERN.match(stripped):
            continue
        
        # Check if this line mentions any header file
        matched_header = None
        for header in header_warnings.keys():
            if header in line:
                matched_header = header
                break
        
        if matched_header:
            header_groups[matched_header].append(line)
        elif "warning:" in line.lower() or "note:" in line.lower():
            # Try to associate with previous header
            pass  # Skip orphan warning lines
        elif "Suppressed" in line or "Use -header-filter" in line:
            # Footer lines - add to all groups
            for header in header_groups:
                if line not in header_groups[header]:
                    header_groups[header].append(line)
    
    # Combine results
    result = {}
    for header, h_warnings in header_warnings.items():
        h_lines = header_groups.get(header, [])
        if task_start_line:
            h_lines = [task_start_line] + h_lines
        result[header] = (h_lines, h_warnings)
    
    return result


def _create_task_content(source_file: str, lines: List[str], warnings: List[Dict], 
                         sub_header: Optional[str] = None) -> str:
    """Create the full content for a task file with summary."""
    header = f"File: {source_file}"
    if sub_header:
        header += f" [Sub: {sub_header}]"
    header += "\n" + "=" * 60 + "\n"
    
    summary = _generate_summary(warnings)
    content = header + summary + "".join(lines)
    
    return content


def split_tidy_logs(log_lines: List[str], output_dir: Path) -> int:
    """
    Parse tidy log lines and create individual task files for files with warnings.

    Args:
        log_lines: List of log lines from ninja tidy output.
        output_dir: Directory to write task_XXX.log files.

    Returns:
        Number of task files created.
    """
    output_dir.mkdir(parents=True, exist_ok=True)

    tasks: Dict[int, Dict] = {}  # task_num -> {file_path, lines}
    current_task: Optional[int] = None
    current_file: Optional[str] = None
    current_lines: List[str] = []

    for line in log_lines:
        match = TASK_START_PATTERN.match(line.strip())

        if match:
            # Save previous task if it exists
            if current_task is not None:
                tasks[current_task] = {
                    "file_path": current_file,
                    "lines": current_lines,
                }

            # Start new task
            current_task = int(match.group(1))
            current_file = match.group(3).strip()
            current_lines = [line]
        elif current_task is not None:
            current_lines.append(line)

    # Save last task
    if current_task is not None:
        tasks[current_task] = {
            "file_path": current_file,
            "lines": current_lines,
        }

    # Process tasks: filter, sub-split large ones, add summaries
    final_tasks = []
    
    for task_num, task_data in tasks.items():
        lines = task_data["lines"]
        warnings = _extract_warnings(lines)
        
        if not warnings:
            continue
        
        line_count = len(lines)
        source_file = task_data["file_path"]
        
        # Check if we need to sub-split
        unique_headers = set(Path(w["file"]).name for w in warnings)
        
        if line_count > LARGE_TASK_THRESHOLD and len(unique_headers) > 1:
            # Sub-split by header file
            header_groups = _group_by_header(lines, warnings)
            
            for header_name, (h_lines, h_warnings) in header_groups.items():
                if h_warnings:  # Only create if there are warnings
                    content = _create_task_content(source_file, h_lines, h_warnings, header_name)
                    final_tasks.append({
                        "source_file": source_file,
                        "sub_header": header_name,
                        "content": content,
                        "size": len(content),
                        "warning_count": len(h_warnings),
                    })
        else:
            # Keep as single task with summary
            content = _create_task_content(source_file, lines, warnings)
            final_tasks.append({
                "source_file": source_file,
                "sub_header": None,
                "content": content,
                "size": len(content),
                "warning_count": len(warnings),
            })

    # Sort by content size (ascending) - smaller/simpler tasks first
    final_tasks.sort(key=lambda x: x["size"])

    # Write task files with sequential IDs
    created_count = 0
    for idx, task_data in enumerate(final_tasks, start=1):
        task_id = f"{idx:03d}"
        task_file = output_dir / f"task_{task_id}.log"
        task_file.write_text(task_data["content"], encoding="utf-8")
        created_count += 1

    return created_count
