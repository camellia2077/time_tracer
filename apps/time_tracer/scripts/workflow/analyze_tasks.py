
import argparse
import os
import re
from collections import defaultdict
import sys

def parse_warning_line(line):
    # Format: path/to/file.hpp:16:38: warning: message [warning-type]
    pattern = r"^(.*?):(\d+):(\d+): warning: (.*?) \[(.*?)\]$"
    match = re.match(pattern, line.strip())
    if match:
        path, line_num, col, message, wtype = match.groups()
        return {
            "path": path,
            "line": int(line_num),
            "col": int(col),
            "message": message,
            "type": wtype,
            "full_line": line.strip()
        }
    return None

def analyze_tasks(start_id, end_id, tasks_dir):
    print(f"Analyzing Task Logs {start_id:03d} - {end_id:03d}...")
    
    warnings_by_file = defaultdict(list)
    processed_count = 0
    
    for i in range(start_id, end_id + 1):
        filename = f"task_{i:03d}.log"
        filepath = os.path.join(tasks_dir, filename)
        
        if not os.path.exists(filepath):
            print(f"Warning: {filename} not found, skipping.")
            continue
            
        processed_count += 1
        with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
            for line in f:
                if ": warning:" in line and "[" in line and "]" in line:
                    warning = parse_warning_line(line)
                    if warning:
                        # Create unique key to deduplicate
                        # key = file + line + type (message might vary slightly, but usually redundant)
                        key = (warning['path'], warning['line'], warning['type'])
                        
                        # Check if duplicate exists
                        exists = False
                        for existing in warnings_by_file[warning['path']]:
                            if (existing['path'], existing['line'], existing['type']) == key:
                                exists = True
                                break
                        
                        if not exists:
                            warnings_by_file[warning['path']].append(warning)

    print(f"Processed {processed_count} logs.")
    print("=" * 60)
    
    total_warnings = 0
    for file_path, warnings in sorted(warnings_by_file.items()):
        print(f"\n[FILE] {file_path}")
        warnings.sort(key=lambda x: x['line'])
        for w in warnings:
            print(f"  L{w['line']}: [{w['type']}] {w['message']}")
            total_warnings += 1
            
    print("=" * 60)
    print(f"Total Unique Warnings: {total_warnings}")

def main():
    parser = argparse.ArgumentParser(description="Aggregate and deduplicate Clang-Tidy task logs.")
    parser.add_argument("start", type=int, help="Start Task ID")
    parser.add_argument("end", type=int, help="End Task ID")
    
    # Locate tasks directory relative to this script
    script_dir = os.path.dirname(os.path.abspath(__file__))
    # Assuming script is in time_tracer_cpp/apps/time_tracer/scripts/workflow
    # Tasks are in time_tracer_cpp/apps/time_tracer/build_tidy/tasks
    default_tasks_dir = os.path.abspath(os.path.join(script_dir, "../../build_tidy/tasks"))
    
    parser.add_argument("--dir", default=default_tasks_dir, help="Tasks directory path")
    
    args = parser.parse_args()
    
    analyze_tasks(args.start, args.end, args.dir)

if __name__ == "__main__":
    main()
