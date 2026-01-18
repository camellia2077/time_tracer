# test/cases/version.py
from pathlib import Path
from ..core.base  import BaseTester, TestCounter
from ..conf.definitions import Colors, TestContext
from ..utils.file_ops import get_folder_size, format_size

# 用于查看exe的version
class VersionChecker(BaseTester):
    """Executes --version command and checks for artifacts."""

    def __init__(self, counter: TestCounter, module_order: int, context: TestContext):
        # 显式接收 context 并传递给父类
        super().__init__(counter, module_order, "version_check", context)

    def run_tests(self) -> bool:
        version_check_success = self.run_command_test(
            test_name="Application Version Check",
            command_args=["--version"],
            add_output_dir=False,
            print_stdout=True
        )

        if not version_check_success:
            return False

        try:
            # [修复] 使用 self.ctx.exe_path 替换原有的 self.executable_path
            if self.ctx.exe_path.exists():
                exe_size = self.ctx.exe_path.stat().st_size
                print(f"    {Colors.GREEN}├─{Colors.RESET} Executable size: {format_size(exe_size)}")
            else:
                print(f"    {Colors.RED}├─{Colors.RESET} Executable not found at: {self.ctx.exe_path}")

            plugins_path = self.ctx.exe_path.parent / "plugins"
            if plugins_path.exists() and plugins_path.is_dir():
                plugins_size = get_folder_size(plugins_path)
                print(f"    {Colors.GREEN}└─{Colors.RESET} Plugins folder size: {format_size(plugins_size)}")
            else:
                print(f"    {Colors.GREEN}└─{Colors.RESET} Plugins folder not found (optional).")

        except Exception as e:
            print(f"    {Colors.RED}Error getting file/folder sizes: {e}{Colors.RESET}")
            pass

        return True