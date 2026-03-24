import subprocess
import sys
from pathlib import Path

_DEFAULT_BUILD_PROCESS_NAMES = ["cmake.exe", "ninja.exe", "ccache.exe"]
_DEFAULT_RUNTIME_LOCK_PROCESS_NAMES = [
    "time_tracer_cli.exe",
    "tc_c_api_smoke_tests.exe",
    "tc_c_api_stability_tests.exe",
    "tt_pipeline_api_tests.exe",
    "tt_query_api_tests.exe",
    "tt_reporting_api_tests.exe",
    "tt_exchange_api_tests.exe",
    "tt_aggregate_runtime_tests.exe",
    "tc_app_aggregate_runtime_smoke_tests.exe",
    "tc_app_pipeline_mod_smoke_tests.exe",
    "tc_app_query_mod_smoke_tests.exe",
    "tc_app_workflow_mod_smoke_tests.exe",
    "tt_android_runtime_shell_smoke_tests.exe",
    "tt_android_runtime_config_tests.exe",
    "tt_android_runtime_reporting_tests.exe",
    "tt_android_runtime_query_tests.exe",
    "tt_android_runtime_pipeline_regression_tests.exe",
    "tt_fmt_parity_tests.exe",
    "tt_file_crypto_tests.exe",
    "tc_query_infra_smoke_tests.exe",
    "tc_reporting_infra_smoke_tests.exe",
    "tc_exchange_infra_smoke_tests.exe",
    "tc_config_infra_smoke_tests.exe",
    "tc_persistence_runtime_infra_smoke_tests.exe",
    "tc_persistence_write_infra_smoke_tests.exe",
    "ttr_tests.exe",
    "ttr_fields_tests.exe",
    "ttr_rt_codec_tests.exe",
]


def _write_stdout_safe(text: str) -> None:
    candidates = [sys.stdout]
    fallback = getattr(sys, "__stdout__", None)
    if fallback is not None and fallback is not sys.stdout:
        candidates.append(fallback)

    for stream in candidates:
        if stream is None:
            continue
        try:
            stream.write(text)
            return
        except UnicodeEncodeError:
            encoding = getattr(stream, "encoding", None) or "utf-8"
            safe_text = text.encode(encoding, errors="replace").decode(encoding, errors="replace")
            try:
                stream.write(safe_text)
                return
            except (BrokenPipeError, OSError, ValueError):
                continue
        except (BrokenPipeError, OSError, ValueError):
            continue


def _taskkill_processes(names: list[str], start_message: str, done_message: str) -> None:
    if not names:
        return
    print(start_message)

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
        print(done_message)
    else:
        print("--- No matching build processes found.")


def kill_build_processes(process_names: list[str] | None = None) -> None:
    names = process_names or _DEFAULT_BUILD_PROCESS_NAMES
    _taskkill_processes(
        names=names,
        start_message="--- Cleaning build processes: cmake/ninja/ccache",
        done_message="--- Build process cleanup complete.",
    )


def kill_runtime_lock_processes(process_names: list[str] | None = None) -> None:
    names = process_names or _DEFAULT_RUNTIME_LOCK_PROCESS_NAMES
    _taskkill_processes(
        names=names,
        start_message=(
            "--- Cleaning runtime lock processes before build "
            "(cli/native tests that may hold core DLL)."
        ),
        done_message="--- Runtime lock process cleanup complete.",
    )


def run_command(
    cmd: list[str],
    cwd: Path | None = None,
    env: dict | None = None,
    log_file: Path | None = None,
    flush_interval: int = 1,
    output_mode: str = "live",
) -> int:
    """
    Runs a command with real-time output to stdout and optional mirroring to a log file.
    """
    normalized_output_mode = (output_mode or "live").strip().lower()
    if normalized_output_mode not in {"live", "quiet"}:
        raise ValueError(f"unsupported output_mode: {output_mode}")

    if normalized_output_mode == "live":
        print(f"--- Running: {' '.join(str(c) for c in cmd)}", flush=True)

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
            if normalized_output_mode == "live":
                _write_stdout_safe(line)
            line_count += 1
            should_flush = (line_count % flush_interval) == 0
            if normalized_output_mode == "live" and should_flush:
                sys.stdout.flush()
            if f:
                f.write(line)
                if should_flush:
                    f.flush()

        if normalized_output_mode == "live":
            sys.stdout.flush()
        if f:
            f.flush()

        return process.wait()
    finally:
        if f:
            f.close()


