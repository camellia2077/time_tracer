
import argparse
import sys
import os
from pathlib import Path

# Add the script directory to path to allow imports from workflow
script_dir = Path(__file__).parent.resolve()
sys.path.append(str(script_dir))

from workflow.task_manager import cleanup_task_logs

def main():
    parser = argparse.ArgumentParser(description="Refactoring Workflow CLI")
    subparsers = parser.add_subparsers(dest="command", help="Command to execute")

    # Clean command
    clean_parser = subparsers.add_parser("clean", help="Clean up task logs")
    clean_parser.add_argument("task_ids", nargs='+', help="Task IDs to remove")

    args = parser.parse_args()

    if args.command == "clean":
        # Define tasks directory relative to this script
        # Script is in .../scripts/refactor.py
        # Tasks are in .../build_tidy/tasks/
        repo_root = script_dir.parent
        tasks_dir = repo_root / "build_tidy" / "tasks"
        
        count = cleanup_task_logs(tasks_dir, args.task_ids)
        print(f"Cleanup finished. Removed {count} log files.")
    else:
        parser.print_help()

if __name__ == "__main__":
    main()
