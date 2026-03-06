import io
import sys
import tempfile
from contextlib import redirect_stdout
from pathlib import Path
from unittest import TestCase

REPO_ROOT = Path(__file__).resolve().parents[3]
SCRIPTS_DIR = REPO_ROOT / "scripts"

if str(SCRIPTS_DIR) not in sys.path:
    sys.path.insert(0, str(SCRIPTS_DIR))

from platform_config.sync import run_generation  # noqa: E402
from platform_paths import windows_cli_config_root  # noqa: E402
from toolchain.commands.cmd_build.common.config_sync import (  # noqa: E402
    resolve_platform_config_output_root,
)
from toolchain.core.context import Context  # noqa: E402


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
                    show_diff=False,
                    allow_overwrite_source=False,
                )
            self.assertEqual(second_ret, 0)
            output = capture.getvalue()
            self.assertIn("cache hit", output.lower())
            self.assertIn('"cache_hit": true', output.lower())

    def test_toolchain_windows_output_root_matches_shared_path_constant(self):
        ctx = Context(REPO_ROOT)
        resolved = resolve_platform_config_output_root(ctx, "windows")
        self.assertEqual(resolved, windows_cli_config_root(REPO_ROOT))
