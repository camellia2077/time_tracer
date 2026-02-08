
#!/usr/bin/env python3
from __future__ import annotations

import subprocess
import sys
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent


def main() -> int:
    workflow_script = SCRIPT_DIR / "workflow.py"
    cmd = [sys.executable, str(workflow_script), *sys.argv[1:]]
    return subprocess.call(cmd)


if __name__ == "__main__":
    raise SystemExit(main())
