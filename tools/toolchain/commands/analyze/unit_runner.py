from __future__ import annotations

import hashlib
import shlex
from pathlib import Path

_WRAPPER_NAMES = {"ccache", "ccache.exe", "sccache", "sccache.exe"}
_STRIP_ARG_NAMES = {
    "-c",
    "-MD",
    "-MMD",
    "-MP",
}
_STRIP_ARG_NAMES_WITH_VALUE = {
    "-o",
    "-MF",
    "-MT",
    "-MQ",
    "-MJ",
    "--serialize-diagnostics",
    "-dependency-file",
}


def format_progress_file(repo_root: Path, file_path: Path) -> str:
    try:
        return file_path.resolve().relative_to(repo_root.resolve()).as_posix()
    except ValueError:
        return file_path.resolve().as_posix()


def parse_compile_command(entry: dict) -> list[str]:
    arguments = entry.get("arguments")
    if isinstance(arguments, list) and arguments:
        return [str(arg) for arg in arguments]
    command_text = entry.get("command")
    if isinstance(command_text, str) and command_text.strip():
        return shlex.split(command_text, posix=False)
    raise ValueError("compile_commands entry is missing both `arguments` and `command`")


def resolve_entry_file(entry: dict) -> Path:
    file_text = str(entry.get("file") or "").strip()
    if not file_text:
        raise ValueError("compile_commands entry is missing `file`")
    file_path = Path(file_text)
    if file_path.is_absolute():
        return file_path.resolve()
    directory_text = str(entry.get("directory") or "").strip()
    base_dir = Path(directory_text) if directory_text else Path.cwd()
    return (base_dir / file_path).resolve()


def is_under_any_root(file_path: Path, roots: list[Path]) -> bool:
    if not roots:
        return True
    file_text = str(file_path).replace("\\", "/").lower()
    for root in roots:
        root_text = str(root).replace("\\", "/").lower().rstrip("/")
        if file_text == root_text or file_text.startswith(root_text + "/"):
            return True
    return False


def is_excluded_analyze_file(file_path: Path, repo_root: Path) -> bool:
    normalized = str(file_path.resolve()).replace("\\", "/").lower()
    if "/_deps/" in normalized:
        return True

    try:
        file_path.resolve().relative_to((repo_root / "out").resolve())
        return True
    except ValueError:
        return False


def strip_build_only_flags(args: list[str]) -> list[str]:
    stripped: list[str] = []
    index = 0
    while index < len(args):
        arg = args[index]
        if arg in _STRIP_ARG_NAMES:
            index += 1
            continue
        if arg in _STRIP_ARG_NAMES_WITH_VALUE:
            index += 2
            continue
        if arg.startswith("/Fo"):
            index += 1
            continue
        stripped.append(arg)
        index += 1
    return stripped


def build_analyzer_command(entry: dict, output_path: Path) -> list[str]:
    raw_args = parse_compile_command(entry)
    if not raw_args:
        raise ValueError("empty compile command")

    prefix: list[str] = []
    index = 0
    while index < len(raw_args) and Path(raw_args[index]).name.lower() in _WRAPPER_NAMES:
        prefix.append(raw_args[index])
        index += 1
    if index >= len(raw_args):
        raise ValueError("compile command does not contain a compiler executable")

    compiler = raw_args[index]
    remaining = strip_build_only_flags(raw_args[index + 1 :])
    return [
        *prefix,
        compiler,
        "--analyze",
        "--analyzer-output",
        "sarif",
        "-o",
        str(output_path),
        *remaining,
    ]


def collect_matched_entries(
    payload: list,
    *,
    repo_root: Path,
    source_roots: list[Path],
) -> list[tuple[dict, Path]]:
    matched_entries: list[tuple[dict, Path]] = []
    for entry in payload:
        if not isinstance(entry, dict):
            continue
        try:
            file_path = resolve_entry_file(entry)
        except ValueError:
            continue
        if is_excluded_analyze_file(file_path, repo_root):
            continue
        if is_under_any_root(file_path, source_roots):
            matched_entries.append((entry, file_path))
    return matched_entries


def run_analysis_units(
    *,
    repo_root: Path,
    matched_entries: list[tuple[dict, Path]],
    unit_reports_dir: Path,
    output_log_path: Path,
    env: dict[str, str],
    build_dir: Path,
    run_subprocess_fn,
) -> tuple[list[Path], list[dict[str, object]], int]:
    report_paths: list[Path] = []
    failed_units: list[dict[str, object]] = []
    analyzed_units = 0
    total_units = len(matched_entries)

    for unit_index, (entry, file_path) in enumerate(matched_entries, start=1):
        digest = hashlib.sha1(str(file_path).encode("utf-8")).hexdigest()[:10]
        unit_report_path = unit_reports_dir / f"unit_{unit_index:05d}_{digest}.sarif"
        display_file = format_progress_file(repo_root, file_path)
        try:
            analyzer_cmd = build_analyzer_command(entry, unit_report_path)
        except ValueError as error:
            print(f"Analyze [{unit_index}/{total_units}] {display_file}", flush=True)
            print(f"Analyze [{unit_index}/{total_units}] skipped: {error}", flush=True)
            failed_units.append(
                {
                    "file": str(file_path),
                    "exit_code": 1,
                    "error": str(error),
                }
            )
            continue

        print(f"Analyze [{unit_index}/{total_units}] {display_file}", flush=True)

        with output_log_path.open("a", encoding="utf-8") as log_handle:
            log_handle.write(f"=== [{unit_index}/{len(matched_entries)}] {file_path}\n")
            log_handle.write("--- Running: " + " ".join(str(arg) for arg in analyzer_cmd) + "\n")

        completed = run_subprocess_fn(
            analyzer_cmd,
            cwd=Path(str(entry.get("directory") or build_dir)),
            env=env,
            stdout=-1,
            stderr=-2,
            text=True,
            encoding="utf-8",
            errors="replace",
            check=False,
        )
        with output_log_path.open("a", encoding="utf-8") as log_handle:
            if completed.stdout:
                log_handle.write(completed.stdout)
                if not completed.stdout.endswith("\n"):
                    log_handle.write("\n")
            log_handle.write(f"--- Exit code: {completed.returncode}\n\n")

        if completed.returncode != 0:
            print(
                f"Analyze [{unit_index}/{total_units}] failed (exit {completed.returncode})",
                flush=True,
            )
            failed_units.append(
                {
                    "file": str(file_path),
                    "exit_code": completed.returncode,
                }
            )
            continue

        analyzed_units += 1
        print(f"Analyze [{unit_index}/{total_units}] done", flush=True)
        if unit_report_path.exists():
            report_paths.append(unit_report_path)

    return report_paths, failed_units, analyzed_units
