# src/tt_testing/core/base.py
import re
from pathlib import Path

from ..conf.definitions import (
    AppExitCode,
    LogRoutingRule,
    SingleTestResult,
    TestContext,
    TestReport,
)
from .executor import CommandExecutor
from .log_routing import LogRoutingManager
from .logger import TestLogger


class TestCounter:
    def __init__(self):
        self.value = 0

    def increment(self):
        self.value += 1
        return self.value


class BaseTester:
    def __init__(
        self,
        counter: TestCounter,
        module_order: int,
        name: str,
        context: TestContext,
        show_output: str = "none",
        log_routing_rules: list[LogRoutingRule] | None = None,
    ):
        self.ctx = context
        self.counter = counter
        self.module_name = name

        log_dir = self.ctx.py_output_base_dir / f"{module_order}_{name}"
        self.logger = TestLogger(log_dir)
        self.log_routing = LogRoutingManager(
            log_dir=self.logger.log_dir,
            module_name=self.module_name,
            rules=log_routing_rules or [],
        )
        self.executor = CommandExecutor(show_output=show_output)

    def run_tests(self) -> TestReport:
        raise NotImplementedError

    def _resolve_log_subdir(self, command_args: list) -> str:
        return self.log_routing.resolve_subdir(command_args)

    def run_command_test(self, test_name: str, command_args: list, **kwargs) -> SingleTestResult:
        current_count = self.counter.increment()
        sanitized_name = re.sub(r"[^a-zA-Z0-9]+", "_", test_name).lower()
        log_file = f"{current_count}_{sanitized_name}.log"
        subdir = self._resolve_log_subdir(command_args)
        if subdir:
            log_file = f"{subdir}/{log_file}"

        raw_command = bool(kwargs.get("raw_command", False))
        if raw_command:
            full_cmd = list(command_args)
        else:
            full_cmd = [str(self.ctx.exe_path)] + command_args
        if kwargs.get("add_output_dir", False):
            full_cmd.extend(["--output", str(self.ctx.output_dir / "exported_files")])

        cwd_path = self.ctx.exe_path.parent
        expect_exit = kwargs.get("expect_exit", 0)
        result = self.executor.run(
            full_cmd,
            input_str=kwargs.get("stdin_input"),
            cwd=cwd_path,
            expected_exit=expect_exit,
        )

        log_path = self.logger.log_result(test_name, log_file, result)

        messages = []
        status = "PASS"

        if expect_exit is not None and result.return_code != expect_exit:
            status = "FAIL"
            err_msg = AppExitCode.to_string(result.return_code)
            messages.append(f"Expected exit {expect_exit}, got {result.return_code} ({err_msg}).")

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

        for spec in kwargs.get("expect_file_contains", []) or []:
            if "::" not in spec:
                status = "FAIL"
                messages.append(f"Invalid expect_file_contains spec (missing '::'): {spec}")
                continue

            path_str, needle = spec.split("::", 1)
            path_obj = Path(path_str)
            if not path_obj.exists():
                status = "FAIL"
                messages.append(f"Missing file for content check: {path_str}")
                continue

            try:
                content = path_obj.read_text(encoding="utf-8", errors="replace")
            except Exception as error:
                status = "FAIL"
                messages.append(f"Failed to read file for content check: {path_str} ({error})")
                continue

            if needle not in content:
                status = "FAIL"
                messages.append(f"Missing file content in {path_str}: {needle}")

        return SingleTestResult(
            name=test_name,
            status=status,
            execution_result=result,
            messages=messages,
            log_file=str(log_path),
        )
