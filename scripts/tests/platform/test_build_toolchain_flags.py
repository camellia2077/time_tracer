import sys
from pathlib import Path
from tempfile import TemporaryDirectory
from unittest import TestCase

REPO_ROOT = Path(__file__).resolve().parents[3]
SCRIPTS_DIR = REPO_ROOT / "scripts"

if str(SCRIPTS_DIR) not in sys.path:
    sys.path.insert(0, str(SCRIPTS_DIR))

from toolchain.commands.cmd_build.common.flags import resolve_toolchain_flags  # noqa: E402
from toolchain.core.context import Context  # noqa: E402


def _write_text(path: Path, content: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content, encoding="utf-8")


class TestBuildToolchainFlags(TestCase):
    def test_compiler_clang_sets_c_and_cxx_compilers(self):
        with TemporaryDirectory() as tmp:
            repo_root = Path(tmp)
            _write_text(
                repo_root / "scripts" / "toolchain" / "config.toml",
                '[build]\ncompiler = "clang"\n',
            )
            ctx = Context(repo_root)

            flags = resolve_toolchain_flags(ctx, [])

            self.assertEqual(
                flags,
                [
                    "-D",
                    "CMAKE_C_COMPILER=clang",
                    "-D",
                    "CMAKE_CXX_COMPILER=clang++",
                ],
            )

    def test_compiler_gcc_sets_c_and_cxx_compilers(self):
        with TemporaryDirectory() as tmp:
            repo_root = Path(tmp)
            _write_text(
                repo_root / "scripts" / "toolchain" / "config.toml",
                '[build]\ncompiler = "gcc"\n',
            )
            ctx = Context(repo_root)

            flags = resolve_toolchain_flags(ctx, [])

            self.assertEqual(
                flags,
                [
                    "-D",
                    "CMAKE_C_COMPILER=gcc",
                    "-D",
                    "CMAKE_CXX_COMPILER=g++",
                ],
            )
