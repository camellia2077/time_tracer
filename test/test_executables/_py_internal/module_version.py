# _py_internal/module_version.py
from .base_module import BaseTester, TestCounter

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
        运行版本检查测试。
        """
        return self.run_command_test(
            test_name="Application Version Check",
            command_args=["--version"],
            add_output_dir=False,
            print_stdout=True  # <--- 在这里启用标准输出的打印功能
        )