import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[4]
TOOLS_DIR = REPO_ROOT / "tools"

if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))


def write_text(path: Path, content: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content, encoding="utf-8")


def write_split_config(repo_root: Path, content: str) -> None:
    write_text(repo_root / "tools" / "toolchain" / "config" / "test.toml", content)


def _write_text(path: Path, content: str) -> None:
    write_text(path, content)


def _write_split_config(repo_root: Path, content: str) -> None:
    write_split_config(repo_root, content)
