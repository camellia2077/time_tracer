import sys
from contextlib import redirect_stdout
from io import StringIO
from pathlib import Path
from tempfile import TemporaryDirectory
from unittest import TestCase

REPO_ROOT = Path(__file__).resolve().parents[3]
TOOLS_DIR = REPO_ROOT / "tools"

if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.toolchain.core.context import Context  # noqa: E402
from tools.toolchain.commands.tidy import workspace as tidy_workspace  # noqa: E402
from tools.tests.platform._path_assertions import assert_same_paths  # noqa: E402


def _write_text(path: Path, content: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content, encoding="utf-8")


def _write_split_config(repo_root: Path, name: str, content: str) -> None:
    _write_text(repo_root / "tools" / "toolchain" / "config" / name, content)


class TestContextConfigResolution(TestCase):
    def test_uses_toolchain_config(self):
        with TemporaryDirectory() as tmp:
            repo_root = Path(tmp)
            _write_split_config(
                repo_root,
                "apps.toml",
                '[apps.demo]\npath = "apps/new_demo"\n',
            )

            ctx = Context(repo_root)
            self.assertEqual(ctx.get_app_dir("demo"), repo_root / "apps" / "new_demo")

    def test_ignores_non_toolchain_root_config(self):
        with TemporaryDirectory() as tmp:
            repo_root = Path(tmp)
            old_cfg = repo_root / "tools" / "config.toml"

            _write_text(
                old_cfg,
                '[apps.demo]\npath = "apps/legacy_demo"\n',
            )

            ctx = Context(repo_root)
            self.assertEqual(
                ctx.get_app_dir("demo"),
                repo_root / "apps" / "demo",
            )

    def test_loads_split_toolchain_config_files(self):
        with TemporaryDirectory() as tmp:
            repo_root = Path(tmp)
            _write_split_config(
                repo_root,
                "apps.toml",
                '[apps.demo]\npath = "apps/split_demo"\n',
            )

            ctx = Context(repo_root)
            self.assertEqual(ctx.get_app_dir("demo"), repo_root / "apps" / "split_demo")

    def test_split_config_ignores_legacy_toolchain_config(self):
        with TemporaryDirectory() as tmp:
            repo_root = Path(tmp)

            _write_text(
                repo_root / "tools" / "toolchain" / "config.toml",
                '[apps.demo]\npath = "apps/base_demo"\n',
            )
            _write_split_config(
                repo_root,
                "apps.toml",
                '[apps.demo]\npath = "apps/split_demo"\n',
            )

            ctx = Context(repo_root)
            self.assertEqual(ctx.get_app_dir("demo"), repo_root / "apps" / "split_demo")

    def test_legacy_toolchain_config_is_ignored_when_split_files_are_absent(self):
        with TemporaryDirectory() as tmp:
            repo_root = Path(tmp)

            _write_text(
                repo_root / "tools" / "toolchain" / "config.toml",
                '[apps.demo]\npath = "apps/base_demo"\n',
            )

            ctx = Context(repo_root)
            self.assertEqual(ctx.get_app_dir("demo"), repo_root / "apps" / "demo")

    def test_build_profile_extends_merges_parent_lists(self):
        with TemporaryDirectory() as tmp:
            repo_root = Path(tmp)
            _write_split_config(
                repo_root,
                "build.toml",
                """
[build]
default_profile = "child"

[build.profiles._base]
build_dir = "build_fast"
cmake_args = ["-D", "A=1"]

[build.profiles.child]
extends = "_base"
cmake_args = ["-D", "B=2"]
""".strip(),
            )

            ctx = Context(repo_root)
            profile = ctx.config.build.profiles["child"]
            self.assertEqual(profile.build_dir, "build_fast")
            self.assertEqual(profile.cmake_args, ["-D", "A=1", "-D", "B=2"])

    def test_build_profile_extends_cycle_falls_back_to_defaults(self):
        with TemporaryDirectory() as tmp:
            repo_root = Path(tmp)
            cfg = repo_root / "tools" / "toolchain" / "config.toml"

            _write_split_config(
                repo_root,
                "build.toml",
                """
[build.profiles.a]
extends = "b"

[build.profiles.b]
extends = "a"
""".strip(),
            )

            captured = StringIO()
            with redirect_stdout(captured):
                ctx = Context(repo_root)
            output = captured.getvalue()
            self.assertIn("Failed to load config", output)
            self.assertEqual(ctx.config.build.profiles, {})

    def test_app_fixed_build_dir_is_loaded(self):
        with TemporaryDirectory() as tmp:
            repo_root = Path(tmp)
            _write_split_config(
                repo_root,
                "apps.toml",
                """
[apps.demo]
path = "apps/demo"
backend = "gradle"
fixed_build_dir = "build"
""".strip(),
            )

            ctx = Context(repo_root)
            self.assertEqual(ctx.get_app_metadata("demo").fixed_build_dir, "build")

    def test_app_cmake_source_path_overrides_source_dir_only(self):
        with TemporaryDirectory() as tmp:
            repo_root = Path(tmp)
            _write_split_config(
                repo_root,
                "apps.toml",
                """
[apps.demo]
path = "apps/demo_shell"
cmake_source_path = "apps/demo_legacy"
""".strip(),
            )

            ctx = Context(repo_root)
            self.assertEqual(ctx.get_app_dir("demo"), repo_root / "apps" / "demo_shell")
            self.assertEqual(
                ctx.get_app_source_dir("demo"),
                repo_root / "apps" / "demo_legacy",
            )

    def test_empty_cmake_source_path_falls_back_to_app_dir(self):
        with TemporaryDirectory() as tmp:
            repo_root = Path(tmp)
            _write_split_config(
                repo_root,
                "apps.toml",
                """
[apps.demo]
path = "apps/demo_shell"
cmake_source_path = ""
""".strip(),
            )

            ctx = Context(repo_root)
            self.assertEqual(
                ctx.get_app_source_dir("demo"),
                repo_root / "apps" / "demo_shell",
            )

    def test_tidy_source_scope_is_loaded_and_resolved(self):
        with TemporaryDirectory() as tmp:
            repo_root = Path(tmp)
            _write_split_config(
                repo_root,
                "workflow.toml",
                """
[apps.demo]
path = "apps/demo"

[tidy.source_scopes.core_family]
roots = [
  "libs/tracer_core/src",
  "libs/tracer_adapters_io/src",
  "libs/tracer_core_bridge_common/src",
  "libs/tracer_transport/src",
]
tidy_build_dir = "build_tidy_core_family"
prebuild_targets = ["tc_shared_lib", "tc_domain_lib"]
""".strip(),
            )

            ctx = Context(repo_root)
            scope_cfg = ctx.config.tidy.source_scopes["core_family"]
            self.assertEqual(scope_cfg.tidy_build_dir, "build_tidy_core_family")
            self.assertEqual(
                scope_cfg.prebuild_targets,
                ["tc_shared_lib", "tc_domain_lib"],
            )
            self.assertEqual(
                scope_cfg.roots,
                [
                    "libs/tracer_core/src",
                    "libs/tracer_adapters_io/src",
                    "libs/tracer_core_bridge_common/src",
                    "libs/tracer_transport/src",
                ],
            )

            workspace = tidy_workspace.resolve_workspace(ctx, source_scope="core_family")
            self.assertEqual(workspace.source_scope, "core_family")
            self.assertEqual(workspace.build_dir_name, "build_tidy_core_family")
            self.assertEqual(
                workspace.prebuild_targets,
                ["tc_shared_lib", "tc_domain_lib"],
            )
            expected_roots = [
                repo_root / "libs" / "tracer_core" / "src",
                repo_root / "libs" / "tracer_adapters_io" / "src",
                repo_root / "libs" / "tracer_core_bridge_common" / "src",
                repo_root / "libs" / "tracer_transport" / "src",
            ]
            for root in expected_roots:
                root.mkdir(parents=True, exist_ok=True)
            assert_same_paths(
                workspace.source_roots,
                expected_roots,
            )
