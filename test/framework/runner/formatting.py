import os
import subprocess
import sys
from pathlib import Path


def should_run_format_on_success(
    args,
    enable_format_on_success: bool,
) -> bool:
    if not enable_format_on_success:
        return False
    return not getattr(args, "no_format_on_success", False)


def run_clang_format_after_success(repo_root: Path, app_name: str) -> int:
    scripts_entry = repo_root / "scripts" / "run.py"
    if not scripts_entry.exists():
        print(f"Warning: scripts entry not found, skip clang-format: {scripts_entry}")
        return 1

    format_cmd = [
        sys.executable,
        str(scripts_entry),
        "format",
        "--app",
        app_name,
    ]
    print("--- All tests passed. Running clang-format via scripts...")
    completed = subprocess.run(
        format_cmd,
        cwd=repo_root,
        env=os.environ.copy(),
        check=False,
        capture_output=True,
        text=True,
    )
    if completed.returncode == 0:
        print("clang-format status: completed")
    else:
        print(f"clang-format status: failed ({completed.returncode})")
    return int(completed.returncode)
