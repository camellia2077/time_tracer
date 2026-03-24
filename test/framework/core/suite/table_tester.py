from __future__ import annotations

from ..conf.definitions import CommandSpec, TestContext, TestReport
from ..base import BaseTester, TestCounter


class TableTester(BaseTester):
    def __init__(
        self,
        counter: TestCounter,
        module_order: int,
        stage: str,
        context: TestContext,
        commands: list[CommandSpec],
        stop_on_failure: bool,
        show_output: str,
        log_routing_rules=None,
    ):
        super().__init__(
            counter,
            module_order,
            stage,
            context,
            show_output=show_output,
            log_routing_rules=log_routing_rules,
        )
        self.commands = commands
        self.stop_on_failure = stop_on_failure

    def run_tests(self) -> TestReport:
        report = TestReport(module_name=self.module_name)
        for spec in self.commands:
            result = self.run_command_test(
                spec.name,
                spec.args,
                add_output_dir=spec.add_output_dir,
                expect_exit=spec.expect_exit,
                raw_command=spec.raw_command,
                stdin_input=spec.stdin_input,
                expect_files=spec.expect_files,
                expect_file_contains=spec.expect_file_contains,
                expect_stdout_contains=spec.expect_stdout_contains,
                expect_stdout_regex=spec.expect_stdout_regex,
                expect_stdout_any_of=spec.expect_stdout_any_of,
                expect_stderr_contains=spec.expect_stderr_contains,
                expect_error_code=spec.expect_error_code,
                expect_error_category=spec.expect_error_category,
                expect_hints_contains=spec.expect_hints_contains,
                expect_json_fields=spec.expect_json_fields,
            )
            report.results.append(result)
            if self.stop_on_failure and result.status == "FAIL":
                break
        return report
