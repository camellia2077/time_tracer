from pathlib import Path
from tempfile import TemporaryDirectory
from unittest import TestCase

from tools.tests.platform.support.path_assertions import (
    assert_command_path_arg,
    assert_same_path,
    assert_same_paths,
)


class TestPathAssertions(TestCase):
    def test_assert_same_path_accepts_str_and_path_for_existing_directory(self):
        with TemporaryDirectory() as tmp:
            root = Path(tmp)
            target = root / "demo"
            target.mkdir(parents=True, exist_ok=True)

            assert_same_path(str(target), target)

    def test_assert_same_path_accepts_nonexistent_semantic_match(self):
        with TemporaryDirectory() as tmp:
            root = Path(tmp)
            expected = root / "out" / "build" / "demo"
            actual = str(expected)

            assert_same_path(actual, expected)

    def test_assert_same_paths_compares_each_item(self):
        with TemporaryDirectory() as tmp:
            root = Path(tmp)
            expected_roots = [
                root / "libs" / "tracer_core" / "src",
                root / "libs" / "tracer_transport" / "src",
            ]
            for item in expected_roots:
                item.mkdir(parents=True, exist_ok=True)

            assert_same_paths([str(item) for item in expected_roots], expected_roots)

    def test_assert_command_path_arg_checks_specific_token(self):
        with TemporaryDirectory() as tmp:
            root = Path(tmp)
            build_dir = root / "out" / "build" / "demo" / "build_fast"
            build_dir.mkdir(parents=True, exist_ok=True)

            assert_command_path_arg(
                ["cmake", "--build", str(build_dir), "-j"],
                2,
                build_dir,
            )
