import sys
from contextlib import redirect_stdout
from io import StringIO
from pathlib import Path
from tempfile import TemporaryDirectory
from unittest import TestCase

REPO_ROOT = Path(__file__).resolve().parents[3]
SCRIPTS_DIR = REPO_ROOT / "scripts"

if str(SCRIPTS_DIR) not in sys.path:
    sys.path.insert(0, str(SCRIPTS_DIR))

from toolchain.core.context import Context  # noqa: E402


def _write_text(path: Path, content: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content, encoding="utf-8")


class TestContextConfigResolution(TestCase):
    def test_uses_toolchain_config(self):
        with TemporaryDirectory() as tmp:
            repo_root = Path(tmp)
            new_cfg = repo_root / "scripts" / "toolchain" / "config.toml"

            _write_text(
                new_cfg,
                '[apps.demo]\npath = "apps/new_demo"\n',
            )

            ctx = Context(repo_root)
            self.assertEqual(ctx.get_app_dir("demo"), repo_root / "apps" / "new_demo")

    def test_ignores_legacy_scripts_config(self):
        with TemporaryDirectory() as tmp:
            repo_root = Path(tmp)
            old_cfg = repo_root / "scripts" / "config.toml"

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
            split_cfg = repo_root / "scripts" / "toolchain" / "config" / "apps.toml"

            _write_text(
                split_cfg,
                '[apps.demo]\npath = "apps/split_demo"\n',
            )

            ctx = Context(repo_root)
            self.assertEqual(ctx.get_app_dir("demo"), repo_root / "apps" / "split_demo")

    def test_split_config_overrides_base_config(self):
        with TemporaryDirectory() as tmp:
            repo_root = Path(tmp)
            base_cfg = repo_root / "scripts" / "toolchain" / "config.toml"
            split_cfg = repo_root / "scripts" / "toolchain" / "config" / "apps.toml"

            _write_text(
                base_cfg,
                '[apps.demo]\npath = "apps/base_demo"\n',
            )
            _write_text(
                split_cfg,
                '[apps.demo]\npath = "apps/split_demo"\n',
            )

            ctx = Context(repo_root)
            self.assertEqual(ctx.get_app_dir("demo"), repo_root / "apps" / "split_demo")

    def test_warns_when_base_config_duplicates_split_keys(self):
        with TemporaryDirectory() as tmp:
            repo_root = Path(tmp)
            base_cfg = repo_root / "scripts" / "toolchain" / "config.toml"
            split_cfg = repo_root / "scripts" / "toolchain" / "config" / "apps.toml"

            _write_text(
                base_cfg,
                '[apps.demo]\npath = "apps/base_demo"\n',
            )
            _write_text(
                split_cfg,
                '[apps.demo]\npath = "apps/split_demo"\n',
            )

            captured = StringIO()
            with redirect_stdout(captured):
                Context(repo_root)
            output = captured.getvalue()
            self.assertIn("deprecated duplicate config keys", output)
            self.assertIn("apps.demo.path", output)

    def test_no_duplicate_warning_when_keys_do_not_overlap(self):
        with TemporaryDirectory() as tmp:
            repo_root = Path(tmp)
            base_cfg = repo_root / "scripts" / "toolchain" / "config.toml"
            split_cfg = repo_root / "scripts" / "toolchain" / "config" / "apps.toml"

            _write_text(
                base_cfg,
                '[apps.base]\npath = "apps/base_demo"\n',
            )
            _write_text(
                split_cfg,
                '[apps.split]\npath = "apps/split_demo"\n',
            )

            captured = StringIO()
            with redirect_stdout(captured):
                Context(repo_root)
            output = captured.getvalue()
            self.assertNotIn("deprecated duplicate config keys", output)
