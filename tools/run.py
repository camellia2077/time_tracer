#!/usr/bin/env python3
import sys
from pathlib import Path


def _enable_line_buffered_stdio() -> None:
    for stream in (sys.stdout, sys.stderr):
        try:
            stream.reconfigure(line_buffering=True, write_through=True)
        except Exception:
            continue


_enable_line_buffered_stdio()

# Add repository root to path so `tools.*` can be imported consistently.
TOOLS_DIR = Path(__file__).resolve().parent
REPO_ROOT = TOOLS_DIR.parent
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

try:
    from tools.toolchain.cli.dispatch import dispatch_command
    from tools.toolchain.cli.parser import build_parser
    from tools.toolchain.core.context import Context
except ImportError as e:
    print(f"Error: Could not load internal toolchain modules.\n{e}", flush=True)
    sys.exit(1)


def main() -> int:
    ctx = Context(REPO_ROOT)
    parser = build_parser(ctx)
    args = parser.parse_args()
    return dispatch_command(args, ctx)


if __name__ == "__main__":
    sys.exit(main())
