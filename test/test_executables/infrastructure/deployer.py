# test/infrastructure/deployer.py
import shutil
import sys
from pathlib import Path
from typing import List
from conf.definitions import Colors

class ArtifactDeployer:
    """负责文件的搬运"""
    def __init__(self, source_root: Path, target_root: Path):
        self.source = source_root
        self.target = target_root

    def deploy(self, files: List[str], folders: List[str]):
        if not self.source.exists():
             print(f"  {Colors.RED}错误: 源目录不存在: {self.source}{Colors.RESET}")
             sys.exit(1)

        # 复制文件
        for name in files:
            src, dst = self.source / name, self.target / name
            if src.exists():
                shutil.copy(src, dst)
            else:
                print(f"  {Colors.YELLOW}警告: 缺失文件 {name}{Colors.RESET}")
        
        # 复制文件夹
        for folder in folders:
            src, dst = self.source / folder, self.target / folder
            if src.is_dir():
                shutil.copytree(src, dst, dirs_exist_ok=True)