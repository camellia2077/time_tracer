from pathlib import Path

from ...core.context import Context
from ...core.executor import run_command
from ..cmd_build import BuildCommand
from ..tidy import TidyCommand

AUTO_REASON_NO_SUCH_FILE = "auto_no_such_file"
AUTO_REASON_GLOB_MISMATCH = "auto_glob_mismatch"

_LOG_REASON_TOKENS = {
    AUTO_REASON_NO_SUCH_FILE: "no such file or directory",
    AUTO_REASON_GLOB_MISMATCH: "glob mismatch",
}


def ensure_compile_commands(
    ctx: Context,
    app_name: str,
    build_dir: Path,
    build_dir_name: str,
) -> int:
    compile_commands_path = build_dir / "compile_commands.json"
    if compile_commands_path.exists():
        return 0
    print(f"--- tidy-refresh: missing {compile_commands_path}, running tidy configure...")
    return BuildCommand(ctx).configure(
        app_name=app_name,
        tidy=True,
        build_dir_name=build_dir_name,
    )


def run_incremental_tidy(
    ctx: Context,
    build_dir: Path,
    batch_name: str,
    files: list[Path],
    keep_going: bool,
    chunk_size: int = 40,
) -> int:
    if not files:
        print(f"--- tidy-refresh: no incremental files for {batch_name}, skip.")
        return 0

    refresh_dir = build_dir / "refresh" / batch_name
    refresh_dir.mkdir(parents=True, exist_ok=True)
    chunks = chunk_paths(files, chunk_size=chunk_size)
    had_failure = False
    for chunk_index, chunk in enumerate(chunks, start=1):
        cmd = ["clang-tidy", "-p", str(build_dir)] + [str(path) for path in chunk]
        log_path = refresh_dir / f"incremental_tidy_{chunk_index:03d}.log"
        print(
            f"--- tidy-refresh: incremental chunk {chunk_index}/{len(chunks)} ({len(chunk)} files)."
        )
        ret = run_command(
            cmd,
            cwd=ctx.repo_root,
            env=ctx.setup_env(),
            log_file=log_path,
        )
        if ret != 0:
            had_failure = True
            if not keep_going:
                print("--- tidy-refresh: incremental tidy failed and keep_going=false, stop.")
                return ret
    if had_failure:
        print("--- tidy-refresh: incremental tidy finished with errors.")
        return 1
    return 0


def run_full_tidy(
    ctx: Context,
    app_name: str,
    jobs: int | None,
    parse_workers: int | None,
    keep_going: bool,
    build_dir_name: str,
) -> int:
    return TidyCommand(ctx).execute(
        app_name=app_name,
        extra_args=[],
        jobs=jobs,
        parse_workers=parse_workers,
        keep_going=keep_going,
        build_dir_name=build_dir_name,
    )


def chunk_paths(paths: list[Path], chunk_size: int) -> list[list[Path]]:
    if chunk_size <= 0:
        return [paths]
    chunks: list[list[Path]] = []
    for index in range(0, len(paths), chunk_size):
        chunks.append(paths[index : index + chunk_size])
    return chunks


def detect_incremental_auto_reasons(build_dir: Path, batch_name: str) -> list[str]:
    refresh_dir = build_dir / "refresh" / batch_name
    if not refresh_dir.exists():
        return []
    matched_reasons: set[str] = set()
    for log_path in sorted(refresh_dir.glob("incremental_tidy_*.log")):
        matched_reasons.update(_scan_log_reasons(log_path))
    return sorted(matched_reasons)


def detect_build_log_auto_reasons(build_dir: Path) -> list[str]:
    build_log_path = build_dir / "build.log"
    if not build_log_path.exists():
        return []
    matched_reasons = _scan_log_reasons(build_log_path)
    if AUTO_REASON_NO_SUCH_FILE in matched_reasons:
        matched_reasons.remove(AUTO_REASON_NO_SUCH_FILE)
    return sorted(matched_reasons)


def build_log_mtime_ns(build_dir: Path) -> int | None:
    build_log_path = build_dir / "build.log"
    if not build_log_path.exists():
        return None
    try:
        return build_log_path.stat().st_mtime_ns
    except OSError:
        return None


def _scan_log_reasons(log_path: Path) -> set[str]:
    matched_reasons: set[str] = set()
    try:
        with log_path.open("r", encoding="utf-8", errors="replace") as handle:
            for line in handle:
                lower_line = line.lower()
                for reason, token in _LOG_REASON_TOKENS.items():
                    if token in lower_line:
                        matched_reasons.add(reason)
                if len(matched_reasons) == len(_LOG_REASON_TOKENS):
                    return matched_reasons
    except OSError:
        return set()
    return matched_reasons
