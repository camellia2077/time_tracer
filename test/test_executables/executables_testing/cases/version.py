# test/cases/version.py
from ..core.base  import BaseTester, TestCounter
from ..conf.definitions import Colors, TestContext, TestReport
from ..utils.file_ops import get_folder_size, format_size

class VersionChecker(BaseTester):
    def __init__(self, counter: TestCounter, module_order: int, context: TestContext):
        super().__init__(counter, module_order, "version_check", context)

    def run_tests(self) -> TestReport:
        report = TestReport(module_name=self.module_name)
        
        # 1. 执行命令
        # 注意：这里 print_stdout 参数在 BaseTester 中不再直接打印，
        # 但我们可以保留它作为 tag 或者在 ExecutionResult 中处理，或者完全移除
        res = self.run_command_test(
            test_name="Application Version Check",
            command_args=["--version"],
            add_output_dir=False
        )
        
        # 2. 附加额外信息 (File Sizes) 到 messages
        if res.status == "PASS":
            try:
                if self.ctx.exe_path.exists():
                    sz = self.ctx.exe_path.stat().st_size
                    res.messages.append(f"{Colors.GREEN}├─{Colors.RESET} Executable size: {format_size(sz)}")
                
                plugins_path = self.ctx.exe_path.parent / "plugins"
                if plugins_path.exists() and plugins_path.is_dir():
                    psz = get_folder_size(plugins_path)
                    res.messages.append(f"{Colors.GREEN}└─{Colors.RESET} Plugins folder size: {format_size(psz)}")
            except Exception as e:
                res.messages.append(f"Error checking sizes: {e}")

            # 也可以把 version 的 stdout 附加上去
            if res.execution_result.stdout:
                 res.messages.append(f"{Colors.CYAN}Version Output:{Colors.RESET}\n{res.execution_result.stdout.strip()}")

        report.results.append(res)
        return report