import os
import subprocess
import sys
from pathlib import Path


def should_run_format_on_success(
    args,
    enable_format_on_success: bool,
    default_format_on_success: bool,
) -> bool:
    if not enable_format_on_success:
        return False
    override = getattr(args, "format_on_success", None)
    if override is None:
        return default_format_on_success
    return bool(override)


def run_clang_format_after_success(repo_root: Path, app_name: str) -> int:
    tools_entry = repo_root / "tools" / "run.py"
    if not tools_entry.exists():
        print(f"Warning: tools entry not found, skip clang-format: {tools_entry}")
        return 1

    format_cmd = [
        sys.executable,
        str(tools_entry),
        "format",
        "--app",
        app_name,
    ]
    print("--- All tests passed. Running clang-format via tools...")
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
