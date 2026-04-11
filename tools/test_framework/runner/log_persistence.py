from pathlib import Path
from typing import Any

OUTPUT_LOG_NAME = "output.log"
FULL_OUTPUT_LOG_NAME = "output_full.log"
TEMP_OUTPUT_LOG_NAME = "python_output.latest.log"


def collect_case_log_paths(
    logs_root: Path,
    case_records: list[dict[str, Any]],
) -> list[Path]:
    root_resolved = logs_root.resolve()
    seen: set[Path] = set()
    collected: list[Path] = []

    for item in case_records:
        if not isinstance(item, dict):
            continue
        log_file = str(item.get("log_file", "")).strip()
        if not log_file:
            continue

        candidate = (logs_root / log_file).resolve()
        try:
            candidate.relative_to(root_resolved)
        except ValueError:
            continue
        if not candidate.is_file() or candidate in seen:
            continue
        seen.add(candidate)
        collected.append(candidate)

    if collected:
        collected.sort(key=lambda path: path.relative_to(root_resolved).as_posix())
        return collected

    ignored_names = {OUTPUT_LOG_NAME, FULL_OUTPUT_LOG_NAME, TEMP_OUTPUT_LOG_NAME}
    for path in logs_root.rglob("*.log"):
        if not path.is_file():
            continue
        if path.name in ignored_names:
            continue
        resolved = path.resolve()
        if resolved in seen:
            continue
        seen.add(resolved)
        collected.append(resolved)

    collected.sort(key=lambda path: path.relative_to(root_resolved).as_posix())
    return collected


def persist_output_logs(
    logs_root: Path,
    temp_log_path: Path,
    case_records: list[dict[str, Any]],
) -> tuple[Path, Path]:
    concise_output_path = (logs_root / OUTPUT_LOG_NAME).resolve()
    concise_text = temp_log_path.read_text(encoding="utf-8")
    concise_output_path.write_text(concise_text, encoding="utf-8")

    full_output_path = (logs_root / FULL_OUTPUT_LOG_NAME).resolve()
    case_log_paths = collect_case_log_paths(logs_root, case_records)
    with full_output_path.open("w", encoding="utf-8") as handle:
        handle.write("=== Session Output (Concise) ===\n")
        handle.write(concise_text)
        if not concise_text.endswith("\n"):
            handle.write("\n")

        handle.write("\n=== Case Logs (Full Test Content) ===\n")
        for case_log_path in case_log_paths:
            relative_path = case_log_path.relative_to(logs_root).as_posix()
            handle.write(f"\n--- {relative_path} ---\n")
            case_log_content = case_log_path.read_text(encoding="utf-8", errors="ignore")
            handle.write(case_log_content)
            if not case_log_content.endswith("\n"):
                handle.write("\n")

    return concise_output_path, full_output_path
