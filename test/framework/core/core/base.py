# src/tt_testing/core/base.py
import re
from typing import Any

from ..conf.definitions import (
    LogRoutingRule,
    SingleTestResult,
    TestContext,
    TestReport,
)
from .command_expectations import evaluate_command_expectations
from .json_payload import (
    extract_json_payload,
    parse_expected_json_value,
    resolve_json_path,
    tokenize_json_path,
)
from .runtime.executor import CommandExecutor
from .runtime.log_routing import LogRoutingManager
from .runtime.logger import TestLogger


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

    @staticmethod
    def _extract_json_payload(text: str) -> tuple[Any | None, str | None]:
        return extract_json_payload(text)

    @staticmethod
    def _tokenize_json_path(path: str) -> list[str | int]:
        return tokenize_json_path(path)

    @classmethod
    def _resolve_json_path(cls, payload: Any, path: str) -> tuple[bool, Any | None, str | None]:
        return resolve_json_path(payload, path)

    @staticmethod
    def _parse_expected_json_value(raw: str) -> Any:
        return parse_expected_json_value(raw)

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

        stdout_text = CommandExecutor.strip_ansi_codes(result.stdout or "")
        stderr_text = CommandExecutor.strip_ansi_codes(result.stderr or "")
        status, messages = evaluate_command_expectations(
            return_code=result.return_code,
            stdout_text=stdout_text,
            stderr_text=stderr_text,
            expect_exit=expect_exit,
            expectations=kwargs,
        )

        return SingleTestResult(
            name=test_name,
            status=status,
            execution_result=result,
            messages=messages,
            log_file=str(log_path),
        )
