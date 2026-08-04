from __future__ import annotations

import shutil
import subprocess
from dataclasses import dataclass
from pathlib import Path


@dataclass(frozen=True, slots=True)
class ClangFormatResult:
    """Result of formatting one source file with clang-format."""

    returncode: int
    output: str = ""


def resolve_clang_format(path: str | None) -> str | None:
    return shutil.which("clang-format", path=path)


def run_clang_format(
    executable: str,
    file_path: Path,
    *,
    cwd: Path,
    env: dict[str, str],
    check_only: bool,
) -> ClangFormatResult:
    command = [executable, "-style=file"]
    if check_only:
        command.extend(["--dry-run", "--Werror"])
    else:
        command.append("-i")
    command.append(str(file_path))
    completed = subprocess.run(
        command,
        cwd=cwd,
        env=env,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        encoding="utf-8",
        errors="replace",
        check=False,
    )
    return ClangFormatResult(
        returncode=int(completed.returncode),
        output=completed.stdout or "",
    )
