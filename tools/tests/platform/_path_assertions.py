from __future__ import annotations

import os
from collections.abc import Sequence
from pathlib import Path


def _coerce_path(path: str | Path) -> Path:
    return path if isinstance(path, Path) else Path(path)


def paths_refer_to_same_location(actual: str | Path, expected: str | Path) -> bool:
    actual_path = _coerce_path(actual)
    expected_path = _coerce_path(expected)

    if actual_path.exists() and expected_path.exists():
        try:
            return actual_path.samefile(expected_path)
        except OSError:
            pass

    actual_norm = os.path.normcase(str(actual_path.resolve(strict=False)))
    expected_norm = os.path.normcase(str(expected_path.resolve(strict=False)))
    return actual_norm == expected_norm


def assert_same_path(actual: str | Path, expected: str | Path) -> None:
    if paths_refer_to_same_location(actual, expected):
        return

    actual_path = _coerce_path(actual)
    expected_path = _coerce_path(expected)
    raise AssertionError(
        "Paths do not refer to the same location:\n"
        f"actual:   {actual_path}\n"
        f"expected: {expected_path}"
    )


def assert_same_paths(
    actual_paths: Sequence[str | Path],
    expected_paths: Sequence[str | Path],
) -> None:
    if len(actual_paths) != len(expected_paths):
        raise AssertionError(
            "Path list lengths differ:\n"
            f"actual={len(actual_paths)} expected={len(expected_paths)}"
        )

    for index, (actual, expected) in enumerate(zip(actual_paths, expected_paths), start=1):
        try:
            assert_same_path(actual, expected)
        except AssertionError as error:
            raise AssertionError(f"Path list item #{index} differs:\n{error}") from error


def assert_command_path_arg(
    command: Sequence[str],
    index: int,
    expected: str | Path,
) -> None:
    if index >= len(command):
        raise AssertionError(
            f"Command argument index {index} is out of range for command of length {len(command)}."
        )
    assert_same_path(command[index], expected)
