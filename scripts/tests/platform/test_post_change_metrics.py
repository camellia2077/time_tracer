import tempfile
import sys
from pathlib import Path
from unittest import TestCase

REPO_ROOT = Path(__file__).resolve().parents[3]
SCRIPTS_DIR = REPO_ROOT / "scripts"

if str(SCRIPTS_DIR) not in sys.path:
    sys.path.insert(0, str(SCRIPTS_DIR))

from toolchain.commands.cmd_workflow.post_change_metrics import collect_binary_size_metrics
from toolchain.core.context import Context


class TestPostChangeMetrics(TestCase):
    def _write_file(self, path: Path, content: bytes) -> None:
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(content)

    def _write_apps_config(self, repo_root: Path) -> None:
        config_dir = repo_root / "scripts" / "toolchain" / "config"
        config_dir.mkdir(parents=True, exist_ok=True)
        apps_toml = config_dir / "apps.toml"
        apps_toml.write_text(
            "\n".join(
                [
                    "[apps.tracer_core]",
                    'path = "apps/tracer_core"',
                    "",
                    "[apps.tracer_windows_rust_cli]",
                    'path = "apps/tracer_cli/windows/rust_cli"',
                    'backend = "cargo"',
                    "",
                ]
            )
            + "\n",
            encoding="utf-8",
        )

    def test_collect_binary_size_metrics_collects_primary_and_suite_bins(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            self._write_apps_config(repo_root)

            self._write_file(
                repo_root / "apps/tracer_core/build_case/bin/tracer_core.dll",
                b"abc",
            )
            self._write_file(
                repo_root
                / "apps/tracer_cli/windows/rust_cli/build_case/bin/time_tracer_cli.exe",
                b"12345",
            )
            self._write_file(
                repo_root / "apps/tracer_core/build_case/bin/README.txt",
                b"not-a-binary",
            )

            ctx = Context(repo_root)
            metrics = collect_binary_size_metrics(
                ctx=ctx,
                app_name="tracer_core",
                build_dir_name="build_case",
            )

            self.assertEqual(metrics["count"], 2)
            self.assertEqual(metrics["total_bytes"], 8)

            candidate_paths = {item["path"] for item in metrics["candidate_dirs"]}
            self.assertIn("apps/tracer_core/build_case/bin", candidate_paths)
            self.assertIn("apps/tracer_cli/windows/rust_cli/build_case/bin", candidate_paths)

            artifact_paths = {item["path"] for item in metrics["artifacts"]}
            self.assertIn("apps/tracer_core/build_case/bin/tracer_core.dll", artifact_paths)
            self.assertIn(
                "apps/tracer_cli/windows/rust_cli/build_case/bin/time_tracer_cli.exe",
                artifact_paths,
            )

    def test_collect_binary_size_metrics_handles_missing_bins(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            self._write_apps_config(repo_root)
            ctx = Context(repo_root)

            metrics = collect_binary_size_metrics(
                ctx=ctx,
                app_name="tracer_core",
                build_dir_name="build_missing",
            )

            self.assertEqual(metrics["count"], 0)
            self.assertEqual(metrics["total_bytes"], 0)
            self.assertTrue(metrics["candidate_dirs"])
            self.assertTrue(all(item["exists"] is False for item in metrics["candidate_dirs"]))
