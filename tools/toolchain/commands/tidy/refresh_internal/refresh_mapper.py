from pathlib import Path

from .. import analysis_compile_db
from ...shared import tidy as tidy_shared
from ..tasking.task_log import list_task_paths, load_task_record


def normalize_batch_name(batch_id: str) -> str:
    return tidy_shared.normalize_required_batch_name(batch_id)


def collect_batch_files(batch_dir: Path) -> list[Path]:
    task_files = list_task_paths(batch_dir.parent, batch_id=batch_dir.name)
    files_by_key: dict[str, Path] = {}
    for task_path in task_files:
        record = load_task_record(task_path)
        if record.source_file:
            candidate = Path(record.source_file)
            files_by_key[path_key(candidate)] = candidate
        for diagnostic in record.diagnostics:
            if not diagnostic.file:
                continue
            candidate = Path(diagnostic.file)
            files_by_key[path_key(candidate)] = candidate
    return sorted(files_by_key.values(), key=lambda path: path_key(path))


def load_compile_units(compile_commands_path: Path) -> list[Path]:
    return analysis_compile_db.load_compile_units(compile_commands_path)


def resolve_incremental_files(
    touched_files: list[Path],
    compile_units: list[Path],
    app_dir: Path,
    neighbor_scope: str,
) -> list[Path]:
    compile_by_key: dict[str, Path] = {}
    compile_by_dir: dict[str, set[Path]] = {}
    compile_by_module: dict[str, set[Path]] = {}
    for unit in compile_units:
        key = path_key(unit)
        compile_by_key[key] = unit
        dir_key = path_key(unit.parent)
        compile_by_dir.setdefault(dir_key, set()).add(unit)
        module = module_key(unit, app_dir)
        compile_by_module.setdefault(module, set()).add(unit)

    selected: set[Path] = set()
    unresolved: list[Path] = []
    touched_abs: list[Path] = []
    for touched in touched_files:
        touched_abs_path = touched if touched.is_absolute() else app_dir / touched
        touched_abs.append(touched_abs_path)
        resolved = match_compile_unit(
            touched_path=touched_abs_path,
            compile_by_key=compile_by_key,
        )
        if resolved is not None:
            selected.add(resolved)
        else:
            unresolved.append(touched_abs_path)

    unresolved_mapped = 0
    for unresolved_path in unresolved:
        dir_key = path_key(unresolved_path.parent)
        same_dir_units = compile_by_dir.get(dir_key, set())
        if same_dir_units:
            selected.update(same_dir_units)
            unresolved_mapped += len(same_dir_units)

    if unresolved:
        unresolved_count = len(unresolved)
        unresolved_text = ", ".join(str(path) for path in unresolved[:5])
        if unresolved_count > 5:
            unresolved_text += ", ..."
        print(
            "--- tidy-refresh: unresolved non-TU paths "
            f"{unresolved_count}, same-dir fallback added "
            f"{unresolved_mapped} compile units. Samples: {unresolved_text}"
        )

    effective_scope = neighbor_scope if neighbor_scope in {"none", "dir", "module"} else "none"
    if effective_scope == "dir":
        seed_dirs = {path_key(path.parent) for path in touched_abs}
        seed_dirs.update(path_key(path.parent) for path in selected)
        for dir_key in seed_dirs:
            selected.update(compile_by_dir.get(dir_key, set()))
    elif effective_scope == "module":
        seed_modules = {module_key(path, app_dir) for path in touched_abs}
        seed_modules.update(module_key(path, app_dir) for path in selected)
        for module in seed_modules:
            selected.update(compile_by_module.get(module, set()))

    result = sorted(selected, key=lambda path: path_key(path))
    print(
        f"--- tidy-refresh: mapped {len(touched_files)} touched files "
        f"to {len(result)} incremental compile units "
        f"(neighbor_scope={effective_scope})."
    )
    return result


def match_compile_unit(
    touched_path: Path,
    compile_by_key: dict[str, Path],
) -> Path | None:
    direct_key = path_key(touched_path)
    if direct_key in compile_by_key:
        return compile_by_key[direct_key]
    normalized_variant = path_key(Path(str(touched_path).replace("/", "\\")))
    return compile_by_key.get(normalized_variant)


def module_key(file_path: Path, app_dir: Path) -> str:
    absolute_path = file_path if file_path.is_absolute() else app_dir / file_path
    try:
        relative = absolute_path.relative_to(app_dir)
        parts = relative.parts
    except ValueError:
        parts = absolute_path.parts
    if len(parts) >= 2 and parts[0].lower() == "src":
        return f"{parts[0].lower()}/{parts[1].lower()}"
    if parts:
        return parts[0].lower()
    return "."


def path_key(path: Path) -> str:
    normalized = str(path).replace("\\", "/")
    while "//" in normalized:
        normalized = normalized.replace("//", "/")
    return normalized.lower()
