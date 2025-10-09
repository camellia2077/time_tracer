# _py_internal/module_version.py
from pathlib import Path
from .base_module import BaseTester, TestCounter, Colors
from .file_utils import get_folder_size, format_size  # <--- 从新模块导入

class VersionChecker(BaseTester):
    """一个专门用于执行 --version 命令并验证其成功的模块。"""

    def __init__(self, counter: TestCounter, module_order: int, **common_args):
        """
        初始化版本检查器。
        我们从 common_args 中提取所需的参数。
        """
        super().__init__(counter, module_order, "version_check", **common_args)

    def run_tests(self) -> bool:
        """
        运行版本检查测试，并打印可执行文件和插件文件夹的大小。
        """
        version_check_success = self.run_command_test(
            test_name="Application Version Check",
            command_args=["--version"],
            add_output_dir=False,
            print_stdout=True
        )

        if not version_check_success:
            return False

        # --- 功能部分保持不变，现在调用的是导入的函数 ---
        try:
            # 1. 获取并打印 EXE 文件大小
            if self.executable_path.exists():
                exe_size = self.executable_path.stat().st_size
                print(f"    {Colors.GREEN}├─{Colors.RESET} Executable size: {format_size(exe_size)}")
            else:
                print(f"    {Colors.RED}├─{Colors.RESET} Executable not found at: {self.executable_path}")

            # 2. 获取并打印 plugins 文件夹大小
            plugins_path = self.executable_path.parent / "plugins"
            if plugins_path.exists() and plugins_path.is_dir():
                plugins_size = get_folder_size(plugins_path)
                print(f"    {Colors.GREEN}└─{Colors.RESET} Plugins folder size: {format_size(plugins_size)}")
            else:
                print(f"    {Colors.GREEN}└─{Colors.RESET} Plugins folder not found (optional).")

        except Exception as e:
            print(f"    {Colors.RED}Error getting file/folder sizes: {e}{Colors.RESET}")
            pass

        return True