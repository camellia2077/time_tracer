import argparse
import os
import subprocess
import sys
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
sys.path.append(str(SCRIPT_DIR.parent))

from workflow.task_manager import cleanup_task_logs


def run_command(command, cwd=None, description=""):
    print(f"--- Running: {description} ---")
    print(f"Command: {command}")
    try:
        result = subprocess.run(command, cwd=cwd, shell=True, check=True)
        print(f"--- {description} PASSED ---\n")
        return True
    except subprocess.CalledProcessError as e:
        print(f"!!! {description} FAILED (Exit Code: {e.returncode}) !!!\n")
        return False


def main():
    parser = argparse.ArgumentParser(description="Verify (Build) and Clean tasks.")
    parser.add_argument("tasks", nargs="+", help="Task IDs (or Start End range) to verify and clean")
    args = parser.parse_args()

    if len(args.tasks) == 2:
        try:
            start_id = int(args.tasks[0])
            end_id = int(args.tasks[1])
            if start_id < end_id:
                print(f"--- Detected valid range {start_id}-{end_id}, auto-expanding... ---")
                args.tasks = [f"{i:03d}" for i in range(start_id, end_id + 1)]
        except ValueError:
            pass

    scripts_dir = SCRIPT_DIR.parent
    project_root = scripts_dir.parent

    build_script = scripts_dir / "build.py"
    build_cmd = f"python3 \"{build_script}\" fast --build-dir build_fast"
    if not run_command(build_cmd, cwd=project_root, description="Level 1: Fast Build"):
        sys.exit(1)

    tasks_dir = project_root / "build_tidy" / "tasks"
    cleanup_task_logs(tasks_dir, args.tasks)

    print("========================================")
    print("[SUCCESS] BATCH VERIFICATION & CLEANUP COMPLETE")
    print("========================================")


if __name__ == "__main__":
    main()
