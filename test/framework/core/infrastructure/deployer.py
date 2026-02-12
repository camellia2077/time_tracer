# test/infrastructure/deployer.py
import os
import shutil
import sys
from pathlib import Path
from typing import List

from ..conf.definitions import Colors


def _color(text: str) -> str:
    return "" if os.environ.get("TT_TEST_NO_COLOR") == "1" else text


class ArtifactDeployer:
    def __init__(self, source_root: Path, target_root: Path):
        self.source = source_root
        self.target = target_root

    def deploy(self, files: List[str], folders: List[str]):
        if not self.source.exists():
            print(
                f"  {_color(Colors.RED)}Error: source directory not found: "
                f"{self.source}{_color(Colors.RESET)}"
            )
            sys.exit(1)

        for name in files:
            src = self.source / name
            dst = self.target / name
            if src.exists():
                shutil.copy(src, dst)
            else:
                print(
                    f"  {_color(Colors.YELLOW)}Warning: missing file "
                    f"{name}{_color(Colors.RESET)}"
                )

        for folder in folders:
            src = self.source / folder
            dst = self.target / folder
            if src.is_dir():
                shutil.copytree(src, dst, dirs_exist_ok=True)
