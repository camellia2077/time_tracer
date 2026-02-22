import sys
from pathlib import Path
from tempfile import TemporaryDirectory
from unittest import TestCase

REPO_ROOT = Path(__file__).resolve().parents[2]
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
