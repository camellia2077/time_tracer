# test/infrastructure/workspace.py
import shutil  # [新增]
from pathlib import Path
from tempfile import TemporaryDirectory
from typing import Optional
from ..conf.definitions import Colors

class Workspace:
    """负责测试工作区清理的管理"""
    def __init__(self, use_temp: bool = True):
        self.use_temp = use_temp
        self._temp_obj = None
    
    # [修改] 增加 should_clean 参数
    def setup(self, target_dir_override: Optional[Path] = None, should_clean: bool = False) -> Path:
        if self.use_temp:
            self._temp_obj = TemporaryDirectory(prefix="tt_test_")
            root = Path(self._temp_obj.name)
            print(f"  {Colors.GREEN}已创建纯净临时测试环境: {root}{Colors.RESET}")
            return root
        else:
            if not target_dir_override:
                raise ValueError("非临时模式必须提供目标路径 (target_dir_override)")
            
            # [新增] 只有在非临时模式且明确要求清理时，才执行清空操作
            if target_dir_override.exists() and should_clean:
                print(f"  {Colors.YELLOW}正在清理测试环境: {target_dir_override}{Colors.RESET}")
                try:
                    # 遍历删除文件夹内的所有内容，但保留根文件夹本身
                    for item in target_dir_override.iterdir():
                        if item.is_dir():
                            shutil.rmtree(item)
                        else:
                            item.unlink()
                except Exception as e:
                    print(f"  {Colors.RED}清理部分文件失败: {e}{Colors.RESET}")

            target_dir_override.mkdir(parents=True, exist_ok=True)
            print(f"  {Colors.YELLOW}使用固定测试环境: {target_dir_override}{Colors.RESET}")
            return target_dir_override

    def teardown(self):
        if self._temp_obj:
            self._temp_obj.cleanup()