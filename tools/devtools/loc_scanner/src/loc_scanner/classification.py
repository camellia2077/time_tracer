from pathlib import Path

from .config import ComponentConfig


def is_test_path(*, file_path: Path, root: Path, test_directory_names: tuple[str, ...]) -> bool:
    try:
        relative_parts = file_path.relative_to(root).parts[:-1]
    except ValueError:
        relative_parts = file_path.parts[:-1]
    return any(part.lower() in test_directory_names for part in relative_parts)


def resolve_component_root(component: ComponentConfig, workspace_root: Path) -> Path:
    path = Path(component.root)
    if not path.is_absolute():
        path = workspace_root / path
    return path.resolve()


def resolve_result_category(
    *, file_path: Path, component_root: Path, component: ComponentConfig,
    test_directory_names: tuple[str, ...],
) -> str:
    if is_test_path(
        file_path=file_path,
        root=component_root,
        test_directory_names=test_directory_names,
    ):
        return "tests"
    return component.category


def resolve_single_language_category(
    *, file_path: Path, scan_root: Path, test_directory_names: tuple[str, ...],
) -> str:
    if is_test_path(
        file_path=file_path,
        root=scan_root,
        test_directory_names=test_directory_names,
    ):
        return "tests"
    try:
        relative_parts = file_path.relative_to(scan_root).parts[:-1]
    except ValueError:
        relative_parts = file_path.parts[:-1]
    lowered_parts = tuple(part.lower() for part in relative_parts)
    if any(
        current == "src" and next_part == "main"
        for current, next_part in zip(lowered_parts, lowered_parts[1:])
    ):
        return "production"
    return "other"


def resolve_profile_baseline_category(
    *, file_path: Path, scan_root: Path, component: ComponentConfig,
    lang: str, test_directory_names: tuple[str, ...],
) -> str:
    if component.category == "tests":
        return "tests"
    if is_test_path(
        file_path=file_path,
        root=scan_root,
        test_directory_names=test_directory_names,
    ):
        return "tests"
    if (
        component.name == "windows_cli"
        and lang == "rs"
        and file_path.as_posix().lower().endswith(
            "windows/rust/src/commands/testing.rs"
        )
    ):
        return "tests"
    if component.name == "android" and lang == "kt":
        return resolve_single_language_category(
            file_path=file_path,
            scan_root=scan_root,
            test_directory_names=test_directory_names,
        )
    return "production"
