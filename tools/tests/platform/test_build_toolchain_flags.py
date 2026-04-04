import sys
from pathlib import Path
from tempfile import TemporaryDirectory
from unittest import TestCase

REPO_ROOT = Path(__file__).resolve().parents[3]
TOOLS_DIR = REPO_ROOT / "tools"

if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.toolchain.commands.cmd_build.common.flags import resolve_toolchain_flags  # noqa: E402
from tools.toolchain.commands.cmd_build import cmake as build_cmake  # noqa: E402
from tools.toolchain.core.context import Context  # noqa: E402
from tools.tests.platform._path_assertions import (  # noqa: E402
    assert_command_path_arg,
)


def _write_text(path: Path, content: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content, encoding="utf-8")


def _write_split_config(repo_root: Path, content: str) -> None:
    _write_text(repo_root / "tools" / "toolchain" / "config" / "test.toml", content)


class TestBuildToolchainFlags(TestCase):
    def test_compiler_clang_sets_c_and_cxx_compilers(self):
        with TemporaryDirectory() as tmp:
            repo_root = Path(tmp)
            _write_split_config(
                repo_root,
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
            _write_split_config(
                repo_root,
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

    def test_tidy_reconfigure_required_when_compile_db_dir_mismatches(self):
        with TemporaryDirectory() as tmp:
            repo_root = Path(tmp)
            _write_split_config(
                repo_root,
                """
[apps.demo]
path = "apps/demo"

[tidy.source_scopes.core_family]
roots = ["libs/tracer_core/src"]
tidy_build_dir = "build_tidy_core_family"
""".strip(),
            )
            ctx = Context(repo_root)
            build_dir = repo_root / "apps" / "demo" / "build_tidy_core_family"
            build_dir.mkdir(parents=True, exist_ok=True)
            cache_text = "\n".join(
                [
                    "TT_CLANG_TIDY_HEADER_FILTER:STRING=^(?!.*[\\\\/]_deps[\\\\/]).*",
                    "TT_CLANG_TIDY_SOURCE_SCOPE:STRING=core_family",
                    "TT_CLANG_TIDY_SOURCE_ROOTS:STRING="
                    + str((repo_root / "libs" / "tracer_core" / "src").resolve()).replace("\\", "/"),
                    "TT_ANALYSIS_COMPILE_DB_DIR:STRING=C:/tmp/old_compile_db",
                    "TT_ENABLE_CXX_CLANG_TIDY_WRAPPER:BOOL=ON",
                ]
            )
            _write_text(build_dir / "CMakeCache.txt", cache_text)

            self.assertTrue(
                build_cmake.needs_tidy_filter_reconfigure(
                    ctx=ctx,
                    tidy=True,
                    build_dir=build_dir,
                    source_scope="core_family",
                )
            )

    def test_tidy_configure_passes_source_targets_cache_arg(self):
        with TemporaryDirectory() as tmp:
            repo_root = Path(tmp)
            _write_split_config(
                repo_root,
                """
[apps.demo]
path = "apps/demo"

[tidy.source_scopes.core_family]
roots = ["libs/tracer_core/src"]
tidy_build_dir = "build_tidy_core_family"
prebuild_targets = ["tc_shared_lib", "tc_app_lib"]
""".strip(),
            )
            ctx = Context(repo_root)
            app_dir = repo_root / "apps" / "demo"
            app_dir.mkdir(parents=True, exist_ok=True)

            captured: list[list[str]] = []

            def _capture_run(command: list[str], env=None, **_kwargs):
                captured.append(command)
                return 0

            ret = build_cmake.configure_cmake(
                ctx=ctx,
                app_name="demo",
                tidy=True,
                source_scope="core_family",
                config_file=None,
                strict_config=False,
                extra_args=None,
                cmake_args=None,
                build_dir_name="build_tidy_core_family",
                profile_name=None,
                resolve_build_dir_name_fn=lambda tidy, build_dir_name, profile_name, app_name: (
                    build_dir_name or "build_tidy_core_family"
                ),
                run_command_fn=_capture_run,
            )

            self.assertEqual(ret, 0)
            self.assertEqual(len(captured), 1)
            self.assertIn(
                "TT_CLANG_TIDY_SOURCE_TARGETS=tc_shared_lib;tc_app_lib",
                captured[0],
            )

    def test_tidy_reconfigure_required_when_target_wrapper_is_missing(self):
        with TemporaryDirectory() as tmp:
            repo_root = Path(tmp)
            _write_split_config(
                repo_root,
                """
[apps.demo]
path = "apps/demo"

[tidy.source_scopes.core_family]
roots = ["libs/tracer_core/src"]
tidy_build_dir = "build_tidy_core_family"
""".strip(),
            )
            ctx = Context(repo_root)
            build_dir = repo_root / "apps" / "demo" / "build_tidy_core_family"
            build_dir.mkdir(parents=True, exist_ok=True)
            cache_text = "\n".join(
                [
                    "TT_CLANG_TIDY_HEADER_FILTER:STRING=^(?!.*[\\\\/]_deps[\\\\/]).*",
                    "TT_CLANG_TIDY_SOURCE_SCOPE:STRING=core_family",
                    "TT_CLANG_TIDY_SOURCE_ROOTS:STRING="
                    + str((repo_root / "libs" / "tracer_core" / "src").resolve()).replace("\\", "/"),
                    "TT_ANALYSIS_COMPILE_DB_DIR:STRING="
                    + str((build_dir / "analysis_compile_db").resolve()).replace("\\", "/"),
                ]
            )
            _write_text(build_dir / "CMakeCache.txt", cache_text)

            self.assertTrue(
                build_cmake.needs_tidy_filter_reconfigure(
                    ctx=ctx,
                    tidy=True,
                    build_dir=build_dir,
                    source_scope="core_family",
                )
            )

    def test_tidy_configure_strips_conflicting_profile_clang_tidy_flags(self):
        with TemporaryDirectory() as tmp:
            repo_root = Path(tmp)
            _write_split_config(
                repo_root,
                """
[apps.demo]
path = "apps/demo"

[build]
default_profile = "fast"

[build.profiles.fast]
cmake_args = [
  "-D", "ENABLE_CLANG_TIDY=OFF",
  "-D", "ENABLE_PCH=ON",
]
""".strip(),
            )
            ctx = Context(repo_root)
            app_dir = repo_root / "apps" / "demo"
            app_dir.mkdir(parents=True, exist_ok=True)

            captured: list[list[str]] = []

            def _capture_run(command: list[str], env=None, **_kwargs):
                captured.append(command)
                return 0

            ret = build_cmake.configure_cmake(
                ctx=ctx,
                app_name="demo",
                tidy=True,
                source_scope=None,
                config_file=None,
                strict_config=False,
                extra_args=None,
                cmake_args=None,
                build_dir_name="build_tidy",
                profile_name=None,
                resolve_build_dir_name_fn=lambda tidy, build_dir_name, profile_name, app_name: (
                    build_dir_name or "build_tidy"
                ),
                run_command_fn=_capture_run,
            )

            self.assertEqual(ret, 0)
            self.assertEqual(len(captured), 1)
            command = captured[0]
            self.assertIn("ENABLE_CLANG_TIDY=ON", command)
            self.assertIn("ENABLE_PCH=OFF", command)
            self.assertIn("TT_ENABLE_CXX_CLANG_TIDY_WRAPPER=OFF", command)
            self.assertNotIn("ENABLE_CLANG_TIDY=OFF", command)
            self.assertNotIn("ENABLE_PCH=ON", command)

    def test_profile_build_does_not_force_reconfigure_when_already_configured(self):
        with TemporaryDirectory() as tmp:
            repo_root = Path(tmp)
            _write_split_config(
                repo_root,
                """
[apps.demo]
path = "apps/demo"
""".strip(),
            )
            ctx = Context(repo_root)
            app_dir = repo_root / "apps" / "demo"
            build_dir = repo_root / "out" / "build" / "demo" / "build_fast"
            app_dir.mkdir(parents=True, exist_ok=True)
            build_dir.mkdir(parents=True, exist_ok=True)

            configure_calls: list[dict] = []
            build_calls: list[list[str]] = []

            def _configure(**kwargs):
                configure_calls.append(kwargs)
                return 0

            def _run(command: list[str], env=None, **_kwargs):
                build_calls.append(command)
                return 0

            ret = build_cmake.build_cmake(
                ctx=ctx,
                app_name="demo",
                tidy=False,
                source_scope=None,
                config_file=None,
                strict_config=False,
                extra_args=None,
                cmake_args=None,
                build_dir_name="build_fast",
                profile_name="fast",
                runtime_platform=None,
                resolve_build_dir_name_fn=lambda tidy, build_dir_name, profile_name, app_name: (
                    build_dir_name or "build_fast"
                ),
                is_configured_fn=lambda *args, **kwargs: True,
                needs_windows_config_reconfigure_fn=lambda app_name, build_dir: False,
                configure_fn=_configure,
                sync_windows_runtime_config_copy_if_needed_fn=lambda **kwargs: 0,
                run_command_fn=_run,
            )

            self.assertEqual(ret, 0)
            self.assertEqual(configure_calls, [])
            self.assertEqual(len(build_calls), 1)
            self.assertEqual(build_calls[0][0:2], ["cmake", "--build"])
            assert_command_path_arg(build_calls[0], 2, build_dir)
            self.assertEqual(build_calls[0][3:], ["-j"])

    def test_runtime_platform_windows_adds_core_runtime_targets(self):
        with TemporaryDirectory() as tmp:
            repo_root = Path(tmp)
            _write_split_config(
                repo_root,
                """
[apps.tracer_core]
path = "apps/tracer_core_shell"
""".strip(),
            )
            ctx = Context(repo_root)
            app_dir = repo_root / "apps" / "tracer_core_shell"
            build_dir = repo_root / "out" / "build" / "tracer_core_shell" / "build"
            app_dir.mkdir(parents=True, exist_ok=True)
            build_dir.mkdir(parents=True, exist_ok=True)

            build_calls: list[list[str]] = []

            def _run(command: list[str], env=None, **_kwargs):
                build_calls.append(command)
                return 0

            ret = build_cmake.build_cmake(
                ctx=ctx,
                app_name="tracer_core",
                tidy=False,
                source_scope=None,
                config_file=None,
                strict_config=False,
                extra_args=None,
                cmake_args=None,
                build_dir_name="build",
                profile_name="release_bundle",
                runtime_platform="windows",
                resolve_build_dir_name_fn=lambda tidy, build_dir_name, profile_name, app_name: (
                    build_dir_name or "build"
                ),
                is_configured_fn=lambda *args, **kwargs: True,
                needs_windows_config_reconfigure_fn=lambda app_name, build_dir: False,
                configure_fn=lambda **kwargs: 0,
                sync_windows_runtime_config_copy_if_needed_fn=lambda **kwargs: 0,
                run_command_fn=_run,
            )

            self.assertEqual(ret, 0)
            self.assertEqual(len(build_calls), 1)
            self.assertEqual(build_calls[0][0:2], ["cmake", "--build"])
            assert_command_path_arg(build_calls[0], 2, build_dir)
            self.assertEqual(
                build_calls[0][3:],
                ["-j", "--target", "tc_rpt_shared_lib", "tc_shared_dll"],
            )

    def test_tidy_reconfigure_required_when_config_file_mismatches(self):
        with TemporaryDirectory() as tmp:
            repo_root = Path(tmp)
            _write_split_config(
                repo_root,
                """
[apps.demo]
path = "apps/demo"
""".strip(),
            )
            ctx = Context(repo_root)
            build_dir = repo_root / "apps" / "demo" / "build_tidy"
            build_dir.mkdir(parents=True, exist_ok=True)
            cache_text = "\n".join(
                [
                    "TT_CLANG_TIDY_HEADER_FILTER:STRING=^(?!.*[\\\\/]_deps[\\\\/]).*",
                    "TT_CLANG_TIDY_CONFIG_FILE:STRING=C:/tmp/old/.clang-tidy",
                    "TT_ANALYSIS_COMPILE_DB_DIR:STRING="
                    + str((build_dir / "analysis_compile_db").resolve()).replace("\\", "/"),
                    "TT_ENABLE_CXX_CLANG_TIDY_WRAPPER:BOOL=OFF",
                ]
            )
            _write_text(build_dir / "CMakeCache.txt", cache_text)

            self.assertTrue(
                build_cmake.needs_tidy_filter_reconfigure(
                    ctx=ctx,
                    tidy=True,
                    build_dir=build_dir,
                    source_scope=None,
                    config_file=".clang-tidy.strict",
                    strict_config=False,
                )
            )

    def test_tidy_configure_passes_explicit_config_file_cache_arg(self):
        with TemporaryDirectory() as tmp:
            repo_root = Path(tmp)
            _write_split_config(
                repo_root,
                """
[apps.demo]
path = "apps/demo"
""".strip(),
            )
            ctx = Context(repo_root)
            app_dir = repo_root / "apps" / "demo"
            app_dir.mkdir(parents=True, exist_ok=True)

            captured: list[list[str]] = []

            def _capture_run(command: list[str], env=None, **_kwargs):
                captured.append(command)
                return 0

            ret = build_cmake.configure_cmake(
                ctx=ctx,
                app_name="demo",
                tidy=True,
                source_scope=None,
                config_file=".clang-tidy.strict",
                strict_config=False,
                extra_args=None,
                cmake_args=None,
                build_dir_name="build_tidy",
                profile_name=None,
                resolve_build_dir_name_fn=lambda tidy, build_dir_name, profile_name, app_name: (
                    build_dir_name or "build_tidy"
                ),
                run_command_fn=_capture_run,
            )

            self.assertEqual(ret, 0)
            self.assertEqual(len(captured), 1)
            self.assertIn(
                "TT_CLANG_TIDY_CONFIG_FILE="
                + str((repo_root / ".clang-tidy.strict").resolve()).replace("\\", "/"),
                captured[0],
            )
