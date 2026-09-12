#!/usr/bin/env python3
import os
import shlex
import subprocess
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


def _libs_scope_requested(argv: list[str]) -> bool:
    if not argv or argv[0] != "verify":
        return False

    scopes: list[str] = []
    for index, value in enumerate(argv[1:]):
        if value == "--scope" and index + 2 <= len(argv) - 1:
            scopes.append(argv[index + 2])
        elif value.startswith("--scope="):
            scopes.append(value.split("=", 1)[1])
    return scopes == ["libs"]


def _requested_test_platform(argv: list[str]) -> str:
    for index, value in enumerate(argv):
        if value == "--test-platform" and index + 1 < len(argv):
            return argv[index + 1]
        if value.startswith("--test-platform="):
            return value.split("=", 1)[1]
    return "auto"


def _option_count(argv: list[str], option: str) -> int:
    return sum(
        1
        for value in argv
        if value == option or value.startswith(f"{option}=")
    )


def _windows_path_to_wsl(path: Path) -> str:
    resolved = path.resolve()
    drive = resolved.drive.rstrip(":").lower()
    remainder = resolved.relative_to(resolved.anchor).as_posix()
    return f"/mnt/{drive}/{remainder}"


def _maybe_delegate_libs_to_wsl(argv: list[str]) -> int | None:
    """Run the shared-library verification in Ubuntu when invoked on Windows."""
    test_platform = _requested_test_platform(argv)
    if test_platform == "ubuntu" and not _libs_scope_requested(argv):
        print("Error: `--test-platform ubuntu` currently supports only `--scope libs`.", file=sys.stderr)
        return 2
    if sys.platform != "win32" or test_platform == "windows":
        return None
    if test_platform != "ubuntu" and not _libs_scope_requested(argv):
        return None
    if os.environ.get("TT_LIBS_UBUNTU_ACTIVE") == "1":
        return None
    # Keep local CLI validation in the current process. In particular, an
    # invalid repeated-profile invocation must report its parser/dispatch
    # error even on Windows hosts without WSL installed.
    if _option_count(argv, "--profile") > 1:
        return None

    distro = os.environ.get("TT_LIBS_WSL_DISTRO", "Ubuntu")
    delegated_args = list(argv)
    if "--build-dir" not in delegated_args:
        delegated_args.extend(("--build-dir", "build_libs_ubuntu"))
    else:
        build_dir_index = delegated_args.index("--build-dir")
        if build_dir_index + 1 < len(delegated_args):
            requested_build_dir = delegated_args[build_dir_index + 1]
            if requested_build_dir in {"build_libs", "build_fast"}:
                delegated_args[build_dir_index + 1] = "build_libs_ubuntu"
    repo_path = _windows_path_to_wsl(REPO_ROOT)
    command = " ".join(shlex.quote(value) for value in delegated_args)
    shell_command = (
        f"cd {shlex.quote(repo_path)} && "
        f"export TT_LIBS_UBUNTU_ACTIVE=1 && "
        f"if [ -x .venv/bin/python ]; then .venv/bin/python tools/run.py {command}; "
        f"else python3 tools/run.py {command}; fi"
    )
    print(
        f"--- libs scope: delegating Windows invocation to WSL2 {distro} "
        f"(build dir: {next((delegated_args[i + 1] for i, value in enumerate(delegated_args) if value == '--build-dir'), 'build_libs_ubuntu')})",
        flush=True,
    )
    completed = subprocess.run(
        ["wsl.exe", "--distribution", distro, "--", "bash", "-lc", shell_command],
        cwd=REPO_ROOT,
        check=False,
    )
    return completed.returncode

try:
    from tools.toolchain.cli.dispatch import dispatch_command
    from tools.toolchain.cli.parser import build_parser
    from tools.toolchain.core.context import Context
except ImportError as e:
    print(f"Error: Could not load internal toolchain modules.\n{e}", flush=True)
    sys.exit(1)


def main() -> int:
    delegated_result = _maybe_delegate_libs_to_wsl(sys.argv[1:])
    if delegated_result is not None:
        return delegated_result
    ctx = Context(REPO_ROOT)
    parser = build_parser(ctx)
    args = parser.parse_args()
    return dispatch_command(args, ctx)


if __name__ == "__main__":
    sys.exit(main())
