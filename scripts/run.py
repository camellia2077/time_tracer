#!/usr/bin/env python3
import sys
from pathlib import Path

# Add current directory to path
SCRIPT_DIR = Path(__file__).resolve().parent
sys.path.append(str(SCRIPT_DIR))

try:
    from toolchain.cli.dispatch import dispatch_command
    from toolchain.cli.parser import build_parser
    from toolchain.core.context import Context
except ImportError as e:
    print(f"Error: Could not load internal toolchain modules.\n{e}")
    sys.exit(1)


def main() -> int:
    repo_root = SCRIPT_DIR.parent
    ctx = Context(repo_root)
    parser = build_parser(ctx)
    args = parser.parse_args()
    return dispatch_command(args, ctx)


if __name__ == "__main__":
    sys.exit(main())
