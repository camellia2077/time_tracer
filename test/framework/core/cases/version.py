# test/cases/version.py
from ..conf.definitions import TestContext, TestReport
from ..core.base import BaseTester, TestCounter
from ..utils.file_ops import format_size, get_folder_size


class VersionChecker(BaseTester):
    def __init__(self, counter: TestCounter, module_order: int,
                 context: TestContext, show_output: str = "none"):
        super().__init__(
            counter,
            module_order,
            "version_check",
            context,
            show_output=show_output,
        )

    def run_tests(self) -> TestReport:
        report = TestReport(module_name=self.module_name)
        result = self.run_command_test(
            test_name="Application Version Check",
            command_args=["--version"],
            add_output_dir=False,
        )

        if result.status == "PASS":
            try:
                if self.ctx.exe_path.exists():
                    size_bytes = self.ctx.exe_path.stat().st_size
                    result.messages.append(
                        f"Executable size: {format_size(size_bytes)}")

                exe_dir = self.ctx.exe_path.parent
                dll_size = 0
                dll_count = 0
                for item in exe_dir.glob("*.dll"):
                    dll_size += item.stat().st_size
                    dll_count += 1

                if dll_count > 0:
                    result.messages.append(
                        f"DLLs size ({dll_count} files): {format_size(dll_size)}")

                plugins_path = exe_dir / "plugins"
                if plugins_path.exists() and plugins_path.is_dir():
                    plugins_size = get_folder_size(plugins_path)
                    result.messages.append(
                        f"Plugins folder size: {format_size(plugins_size)}")
            except Exception as error:
                result.messages.append(f"Error checking sizes: {error}")

            if result.execution_result.stdout:
                version_text = result.execution_result.stdout.strip()
                result.messages.append(f"[Version Output]\n{version_text}")

        report.results.append(result)
        return report
