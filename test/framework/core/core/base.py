# src/tt_testing/core/base.py
import re
from pathlib import Path

from ..conf.definitions import AppExitCode, SingleTestResult, TestContext, TestReport
from .executor import CommandExecutor
from .logger import TestLogger


class TestCounter:
    def __init__(self):
        self.value = 0

    def increment(self):
        self.value += 1
        return self.value


class BaseTester:
    def __init__(self, counter: TestCounter, module_order: int, name: str,
                 context: TestContext, show_output: str = "none"):
        self.ctx = context
        self.counter = counter
        self.module_name = name

        log_dir = self.ctx.py_output_base_dir / f"{module_order}_{name}"
        self.logger = TestLogger(log_dir)
        self.executor = CommandExecutor(show_output=show_output)

    def run_tests(self) -> TestReport:
        raise NotImplementedError

    def run_command_test(self, test_name: str, command_args: list,
                         **kwargs) -> SingleTestResult:
        current_count = self.counter.increment()
        sanitized_name = re.sub(r"[^a-zA-Z0-9]+", "_", test_name).lower()
        log_file = f"{current_count}_{sanitized_name}.log"

        full_cmd = [str(self.ctx.exe_path)] + command_args
        if kwargs.get("add_output_dir", False):
            full_cmd.extend(["--output", str(self.ctx.output_dir /
                                             "exported_files")])

        cwd_path = self.ctx.exe_path.parent
        result = self.executor.run(
            full_cmd,
            input_str=kwargs.get("stdin_input"),
            cwd=cwd_path,
        )

        log_path = self.logger.log_result(test_name, log_file, result)

        messages = []
        status = "PASS"

        expect_exit = kwargs.get("expect_exit", 0)
        if expect_exit is not None and result.return_code != expect_exit:
            status = "FAIL"
            err_msg = AppExitCode.to_string(result.return_code)
            messages.append(
                f"Expected exit {expect_exit}, got {result.return_code} ({err_msg}).")

        stdout_text = CommandExecutor.strip_ansi_codes(result.stdout or "")
        stderr_text = CommandExecutor.strip_ansi_codes(result.stderr or "")

        for needle in kwargs.get("expect_stdout_contains", []) or []:
            if needle not in stdout_text:
                status = "FAIL"
                messages.append(f"Missing stdout text: {needle}")

        for needle in kwargs.get("expect_stderr_contains", []) or []:
            if needle not in stderr_text:
                status = "FAIL"
                messages.append(f"Missing stderr text: {needle}")

        for path_str in kwargs.get("expect_files", []) or []:
            if not Path(path_str).exists():
                status = "FAIL"
                messages.append(f"Missing file: {path_str}")

        return SingleTestResult(
            name=test_name,
            status=status,
            execution_result=result,
            messages=messages,
            log_file=str(log_path),
        )
