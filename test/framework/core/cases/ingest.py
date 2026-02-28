# test/cases/ingest.py
from ..conf.definitions import TestContext, TestReport
from ..core.base import BaseTester, TestCounter


class IngestTester(BaseTester):
    def __init__(
        self,
        counter: TestCounter,
        module_order: int,
        context: TestContext,
        specific_validation_path=None,
    ):
        super().__init__(counter, module_order, "ingest", context)

    def run_tests(self) -> TestReport:
        report = TestReport(module_name=self.module_name)

        source_path_str = str(self.ctx.source_data_path)
        tests_to_run = [
            {
                "name": "Full Pipeline (Blink)",
                "args": ["ingest", source_path_str],
                "add_output": True,
            }
        ]

        for test in tests_to_run:
            res = self.run_command_test(
                test["name"], test["args"], add_output_dir=test["add_output"]
            )
            report.results.append(res)

        return report
