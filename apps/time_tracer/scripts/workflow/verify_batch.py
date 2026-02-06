
import argparse
import subprocess
import sys
import os
import platform

def run_command(command, cwd=None, description=""):
    print(f"--- Running: {description} ---")
    print(f"Command: {command}")
    try:
        # Use shell=True for complex commands (like python calls on Windows)
        result = subprocess.run(command, cwd=cwd, shell=True, check=True)
        print(f"--- {description} PASSED ---\n")
        return True
    except subprocess.CalledProcessError as e:
        print(f"!!! {description} FAILED (Exit Code: {e.returncode}) !!!\n")
        return False

def main():
    parser = argparse.ArgumentParser(description="Verify (Build+Test) and Clean tasks.")
    parser.add_argument("tasks", nargs='+', help="Task IDs (or Start End range) to verify and clean")
    args = parser.parse_args()

    # Smart Range Expansion: If 2 args provided, treat as Start..End range
    if len(args.tasks) == 2:
        try:
            start_id = int(args.tasks[0])
            end_id = int(args.tasks[1])
            if start_id < end_id:
                print(f"--- Detected valid range {start_id}-{end_id}, auto-expanding... ---")
                args.tasks = [f"{i:03d}" for i in range(start_id, end_id + 1)]
        except ValueError:
            # Not integers, treat as normal list
            pass
    
    # Root of the repo (Assumed based on script location)
    script_dir = os.path.dirname(os.path.abspath(__file__))
    # Script is in .../time_tracer_cpp/apps/time_tracer/scripts/workflow
    # Root is .../time_tracer (5 levels up)
    repo_root = os.path.abspath(os.path.join(script_dir, "../../../../../"))
    
    # Scripts dir (parent of workflow)
    scripts_dir = os.path.abspath(os.path.join(script_dir, "../"))

    # 1. Build Verification (Fast)
    build_script = os.path.join(scripts_dir, "build.py")
    # Wrap in python call
    build_cmd = f"python3 \"{build_script}\" --build-dir build_fast --no-tidy --no-opt --no-lto"
    
    if not run_command(build_cmd, cwd=repo_root, description="Level 1: Fast Build"):
        sys.exit(1)

    # 2. Regression Test (Full)
    # Windows batch file execution
    test_cmd = "cmd /c \"cd my_test/test_executables && echo N | run_fast.bat\""
    
    if not run_command(test_cmd, cwd=repo_root, description="Level 2: Regression Test"):
        sys.exit(1)

    # 3. Cleanup
    refactor_script = os.path.join(scripts_dir, "refactor.py")
    ids_str = " ".join(args.tasks)
    clean_cmd = f"python3 \"{refactor_script}\" clean {ids_str}"
    
    if not run_command(clean_cmd, cwd=repo_root, description="Cleanup Tasks"):
        sys.exit(1)

    print("========================================")
    print("[SUCCESS] BATCH VERIFICATION & CLEANUP COMPLETE")
    print("========================================")

if __name__ == "__main__":
    main()
