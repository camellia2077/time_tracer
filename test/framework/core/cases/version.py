# test/cases/version.py
from ..conf.definitions import CommandSpec, TestContext, TestReport
from ..core.base import BaseTester, TestCounter
from ..utils.file_ops import format_size, get_folder_size


class VersionChecker(BaseTester):
    def __init__(
        self,
        counter: TestCounter,
        module_order: int,
        context: TestContext,
        commands: list[CommandSpec] | None = None,
        show_output: str = "none",
        log_routing_rules=None,
    ):
        super().__init__(
            counter,
            module_order,
            "version_check",
            context,
            show_output=show_output,
            log_routing_rules=log_routing_rules,
        )
        self.commands = list(commands) if commands else []

    @staticmethod
    def _is_version_flag(args: list[str]) -> bool:
        return args == ["--version"] or args == ["-v"]

    def _append_version_diagnostics(self, result) -> None:
        try:
            if self.ctx.exe_path.exists():
                size_bytes = self.ctx.exe_path.stat().st_size
                result.messages.append(f"Executable size: {format_size(size_bytes)}")

            exe_dir = self.ctx.exe_path.parent
            dll_size = 0
            dll_count = 0
            for item in exe_dir.glob("*.dll"):
                dll_size += item.stat().st_size
                dll_count += 1

            if dll_count > 0:
                result.messages.append(f"DLLs size ({dll_count} files): {format_size(dll_size)}")

            plugins_path = exe_dir / "plugins"
            if plugins_path.exists() and plugins_path.is_dir():
                plugins_size = get_folder_size(plugins_path)
                result.messages.append(f"Plugins folder size: {format_size(plugins_size)}")
        except Exception as error:
            result.messages.append(f"Error checking sizes: {error}")

        if result.execution_result and result.execution_result.stdout:
            version_text = result.execution_result.stdout.strip()
            if version_text:
                result.messages.append(f"[Version Output]\n{version_text}")

    def run_tests(self) -> TestReport:
        report = TestReport(module_name=self.module_name)
        commands = self.commands or [
            CommandSpec(
                name="Application Version Check",
                args=["--version"],
                stage="version",
            )
        ]

        for command in commands:
            result = self.run_command_test(
                test_name=command.name,
                command_args=command.args,
                add_output_dir=command.add_output_dir,
                expect_exit=command.expect_exit,
                raw_command=command.raw_command,
                stdin_input=command.stdin_input,
                expect_files=command.expect_files,
                expect_file_contains=command.expect_file_contains,
                expect_stdout_contains=command.expect_stdout_contains,
                expect_stderr_contains=command.expect_stderr_contains,
            )

            if result.status == "PASS" and self._is_version_flag(command.args):
                self._append_version_diagnostics(result)

            report.results.append(result)

        return report
