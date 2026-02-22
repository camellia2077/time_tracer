import subprocess
import sys
from pathlib import Path


def _write_stdout_safe(text: str) -> None:
    try:
        sys.stdout.write(text)
        return
    except UnicodeEncodeError:
        pass

    encoding = sys.stdout.encoding or "utf-8"
    safe_text = text.encode(encoding, errors="replace").decode(encoding, errors="replace")
    sys.stdout.write(safe_text)


def kill_build_processes(process_names: list[str] | None = None) -> None:
    names = process_names or ["cmake.exe", "ninja.exe", "ccache.exe"]
    print("--- Cleaning build processes: cmake/ninja/ccache")

    found_any = False
    for name in names:
        try:
            result = subprocess.run(
                ["taskkill", "/F", "/T", "/IM", name],
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
                encoding="utf-8",
                errors="replace",
                check=False,
            )
        except FileNotFoundError:
            print("--- taskkill is not available. Skipping process cleanup.")
            return

        output = (result.stdout or "").strip()
        if result.returncode == 0:
            found_any = True
            continue

        lowered = output.lower()
        if "not found" in lowered or "找不到" in output or "没有找到" in output:
            continue
        if output:
            safe_output = output.encode("gbk", errors="ignore").decode("gbk", errors="ignore")
            if safe_output:
                print(f"--- taskkill {name} returned {result.returncode}: {safe_output}")
            else:
                print(f"--- taskkill {name} returned {result.returncode} (non-gbk output omitted)")

    if found_any:
        print("--- Build process cleanup complete.")
    else:
        print("--- No matching build processes found.")


def run_command(
    cmd: list[str],
    cwd: Path | None = None,
    env: dict | None = None,
    log_file: Path | None = None,
    flush_interval: int = 20,
) -> int:
    """
    Runs a command with real-time output to stdout and optional mirroring to a log file.
    """
    print(f"--- Running: {' '.join(str(c) for c in cmd)}")

    # Ensure stdout is mirrored and line-buffered
    f = None
    if log_file:
        log_file.parent.mkdir(parents=True, exist_ok=True)
        f = open(log_file, "w", encoding="utf-8")

    try:
        flush_interval = 1 if flush_interval <= 0 else flush_interval
        process = subprocess.Popen(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            cwd=cwd,
            env=env,
            text=True,
            encoding="utf-8",
            errors="replace",
            bufsize=1,
        )

        line_count = 0
        for line in process.stdout:
            _write_stdout_safe(line)
            line_count += 1
            should_flush = (line_count % flush_interval) == 0
            if should_flush:
                sys.stdout.flush()
            if f:
                f.write(line)
                if should_flush:
                    f.flush()

        sys.stdout.flush()
        if f:
            f.flush()

        return process.wait()
    finally:
        if f:
            f.close()
