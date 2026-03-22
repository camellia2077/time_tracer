import sys
from pathlib import Path
from tempfile import TemporaryDirectory
from unittest import TestCase

REPO_ROOT = Path(__file__).resolve().parents[3]

if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.toolchain.commands.cmd_build import cargo as build_cargo  # noqa: E402
from tools.toolchain.core.context import Context  # noqa: E402


def _write_text(path: Path, content: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content, encoding="utf-8")


def _write_bytes(path: Path, content: bytes = b"x") -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(content)


def _write_config(repo_root: Path) -> None:
    _write_text(
        repo_root / "tools" / "toolchain" / "config.toml",
        """
[apps.tracer_core]
path = "apps/tracer_core_shell"

[apps.tracer_windows_rust_cli]
path = "apps/cli/windows/rust"
backend = "cargo"
config_sync_target = "windows"
""".strip(),
    )


def _write_runtime_manifest(path: Path, files: list[str]) -> None:
    payload = {
        "runtime": {
            "required_files": files,
        }
    }
    import json

    _write_text(path, json.dumps(payload))


class TestBuildCargo(TestCase):
    def test_build_cargo_relaxed_sync_uses_core_build_fast_fallback(self):
        with TemporaryDirectory() as tmp:
            repo_root = Path(tmp)
            _write_config(repo_root)
            ctx = Context(repo_root)

            app_dir = ctx.get_app_dir("tracer_windows_rust_cli")
            _write_bytes(app_dir / "target" / "debug" / "time_tracer_cli.exe")

            core_fast_bin = ctx.get_build_layout("tracer_core", "build_fast").bin_dir
            _write_bytes(core_fast_bin / "tracer_core.dll")
            _write_bytes(core_fast_bin / "reports_shared.dll")
            _write_runtime_manifest(
                core_fast_bin / "runtime_manifest.json",
                ["tracer_core.dll", "reports_shared.dll"],
            )

            ret = build_cargo.build_cargo(
                ctx=ctx,
                app_name="tracer_windows_rust_cli",
                tidy=False,
                extra_args=None,
                cmake_args=None,
                build_dir_name="build",
                profile_name=None,
                rust_runtime_sync="relaxed",
                runtime_platform="windows",
                run_command_fn=lambda *args, **kwargs: 0,
            )

            self.assertEqual(ret, 0)
            runtime_bin = ctx.get_build_layout("tracer_windows_rust_cli", "build").bin_dir
            self.assertTrue((runtime_bin / "tracer_core.dll").exists())
            self.assertTrue((runtime_bin / "reports_shared.dll").exists())

    def test_build_cargo_strict_sync_skips_core_build_fast_fallback(self):
        with TemporaryDirectory() as tmp:
            repo_root = Path(tmp)
            _write_config(repo_root)
            ctx = Context(repo_root)

            app_dir = ctx.get_app_dir("tracer_windows_rust_cli")
            _write_bytes(app_dir / "target" / "debug" / "time_tracer_cli.exe")

            core_fast_bin = ctx.get_build_layout("tracer_core", "build_fast").bin_dir
            _write_bytes(core_fast_bin / "tracer_core.dll")
            _write_bytes(core_fast_bin / "reports_shared.dll")
            _write_runtime_manifest(
                core_fast_bin / "runtime_manifest.json",
                ["tracer_core.dll", "reports_shared.dll"],
            )

            ret = build_cargo.build_cargo(
                ctx=ctx,
                app_name="tracer_windows_rust_cli",
                tidy=False,
                extra_args=None,
                cmake_args=None,
                build_dir_name="build",
                profile_name=None,
                rust_runtime_sync="strict",
                runtime_platform="windows",
                run_command_fn=lambda *args, **kwargs: 0,
            )

            self.assertEqual(ret, 0)
            runtime_bin = ctx.get_build_layout("tracer_windows_rust_cli", "build").bin_dir
            self.assertFalse((runtime_bin / "tracer_core.dll").exists())
            self.assertFalse((runtime_bin / "reports_shared.dll").exists())
