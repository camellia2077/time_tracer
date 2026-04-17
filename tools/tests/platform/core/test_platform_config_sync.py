import io
import sys
import tempfile
from contextlib import redirect_stdout
from pathlib import Path
from unittest import TestCase

REPO_ROOT = Path(__file__).resolve().parents[4]
TOOLS_DIR = REPO_ROOT / "tools"

if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.platform_config.sync import run_generation  # noqa: E402
from tools.platform_paths import windows_cli_config_root  # noqa: E402
from tools.toolchain.commands.cmd_build.common.config_sync import (  # noqa: E402
    resolve_platform_config_output_root,
)
from tools.toolchain.core.context import Context  # noqa: E402


class TestPlatformConfigSync(TestCase):
    def test_windows_sync_writes_state_and_cache_hit(self):
        source_root = REPO_ROOT / "assets" / "tracer_core" / "config"
        with tempfile.TemporaryDirectory() as temp_dir:
            temp_root = Path(temp_dir)
            windows_out = temp_root / "windows_config"
            android_out = temp_root / "android_config"

            first_ret = run_generation(
                target="windows",
                source_root=source_root,
                windows_output_root=windows_out,
                android_output_root=android_out,
                apply=True,
                check=False,
                show_diff=False,
                allow_overwrite_source=False,
            )
            self.assertEqual(first_ret, 0)
            self.assertTrue((windows_out / "meta" / "sync_state.json").exists())
            self.assertTrue((windows_out / "config.toml").exists())

            capture = io.StringIO()
            with redirect_stdout(capture):
                second_ret = run_generation(
                    target="windows",
                    source_root=source_root,
                    windows_output_root=windows_out,
                    android_output_root=android_out,
                    apply=True,
                    check=False,
                    show_diff=False,
                    allow_overwrite_source=False,
                )
            self.assertEqual(second_ret, 0)
            output = capture.getvalue()
            self.assertIn("cache hit", output.lower())
            self.assertIn('"cache_hit": true', output.lower())

    def test_check_mode_passes_when_output_is_current(self):
        source_root = REPO_ROOT / "assets" / "tracer_core" / "config"
        with tempfile.TemporaryDirectory() as temp_dir:
            temp_root = Path(temp_dir)
            windows_out = temp_root / "windows_config"
            android_out = temp_root / "android_config"

            apply_ret = run_generation(
                target="windows",
                source_root=source_root,
                windows_output_root=windows_out,
                android_output_root=android_out,
                apply=True,
                check=False,
                show_diff=False,
                allow_overwrite_source=False,
            )
            self.assertEqual(apply_ret, 0)

            capture = io.StringIO()
            with redirect_stdout(capture):
                check_ret = run_generation(
                    target="windows",
                    source_root=source_root,
                    windows_output_root=windows_out,
                    android_output_root=android_out,
                    apply=False,
                    check=True,
                    show_diff=False,
                    allow_overwrite_source=False,
                )

            self.assertEqual(check_ret, 0)
            self.assertIn("up to date", capture.getvalue().lower())

    def test_check_mode_fails_on_drift_without_writing(self):
        source_root = REPO_ROOT / "assets" / "tracer_core" / "config"
        with tempfile.TemporaryDirectory() as temp_dir:
            temp_root = Path(temp_dir)
            windows_out = temp_root / "windows_config"
            android_out = temp_root / "android_config"

            apply_ret = run_generation(
                target="windows",
                source_root=source_root,
                windows_output_root=windows_out,
                android_output_root=android_out,
                apply=True,
                check=False,
                show_diff=False,
                allow_overwrite_source=False,
            )
            self.assertEqual(apply_ret, 0)

            config_file = windows_out / "config.toml"
            original = config_file.read_text(encoding="utf-8")
            config_file.write_text(original + "\n# drift\n", encoding="utf-8")

            capture = io.StringIO()
            with redirect_stdout(capture):
                check_ret = run_generation(
                    target="windows",
                    source_root=source_root,
                    windows_output_root=windows_out,
                    android_output_root=android_out,
                    apply=False,
                    check=True,
                    show_diff=False,
                    allow_overwrite_source=False,
                )

            self.assertEqual(check_ret, 1)
            self.assertIn("drift detected", capture.getvalue().lower())
            self.assertTrue(config_file.read_text(encoding="utf-8").endswith("# drift\n"))

    def test_check_mode_ignores_toml_line_ending_only_drift(self):
        source_root = REPO_ROOT / "assets" / "tracer_core" / "config"
        with tempfile.TemporaryDirectory() as temp_dir:
            temp_root = Path(temp_dir)
            windows_out = temp_root / "windows_config"
            android_out = temp_root / "android_config"

            apply_ret = run_generation(
                target="android",
                source_root=source_root,
                windows_output_root=windows_out,
                android_output_root=android_out,
                apply=True,
                check=False,
                show_diff=False,
                allow_overwrite_source=False,
            )
            self.assertEqual(apply_ret, 0)

            bundle_file = android_out / "meta" / "bundle.toml"
            bundle_text = bundle_file.read_text(encoding="utf-8")
            bundle_file.write_text(
                bundle_text.replace("\n", "\r\n"),
                encoding="utf-8",
                newline="",
            )

            capture = io.StringIO()
            with redirect_stdout(capture):
                check_ret = run_generation(
                    target="android",
                    source_root=source_root,
                    windows_output_root=windows_out,
                    android_output_root=android_out,
                    apply=False,
                    check=True,
                    show_diff=False,
                    allow_overwrite_source=False,
                )

            self.assertEqual(check_ret, 0)
            self.assertIn("up to date", capture.getvalue().lower())

    def test_toolchain_windows_output_root_matches_shared_path_constant(self):
        ctx = Context(REPO_ROOT)
        resolved = resolve_platform_config_output_root(ctx, "windows")
        self.assertEqual(resolved, windows_cli_config_root(REPO_ROOT))
