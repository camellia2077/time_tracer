#!/usr/bin/env python3
import subprocess
import sys
from pathlib import Path


def main(argv: list[str] | None = None) -> int:
    forwarded = list(sys.argv[1:] if argv is None else argv)
    if forwarded and forwarded[0] == "verify":
        forwarded = forwarded[1:]
    if not forwarded:
        forwarded = ["--help"]

    script_dir = Path(__file__).resolve().parent
    repo_root = script_dir.parent
    run_script = script_dir / "run.py"
    cmd = [sys.executable, str(run_script), "verify", *forwarded]
    completed = subprocess.run(cmd, cwd=str(repo_root), check=False)
    return int(completed.returncode)


if __name__ == "__main__":
    sys.exit(main())
