# test/infrastructure/workspace.py
from pathlib import Path
from tempfile import TemporaryDirectory
from typing import Optional
from conf.definitions import Colors

class Workspace:
    """负责测试工作区清理的管理"""
    def __init__(self, use_temp: bool = True):
        self.use_temp = use_temp
        self._temp_obj = None
    
    def setup(self, target_dir_override: Optional[Path] = None) -> Path:
        if self.use_temp:
            self._temp_obj = TemporaryDirectory(prefix="tt_test_")
            root = Path(self._temp_obj.name)
            print(f"  {Colors.GREEN}已创建纯净临时测试环境: {root}{Colors.RESET}")
            return root
        else:
            if not target_dir_override:
                raise ValueError("非临时模式必须提供目标路径 (target_dir_override)")
            target_dir_override.mkdir(parents=True, exist_ok=True)
            print(f"  {Colors.YELLOW}使用固定测试环境: {target_dir_override}{Colors.RESET}")
            return target_dir_override

    def teardown(self):
        if self._temp_obj:
            self._temp_obj.cleanup()