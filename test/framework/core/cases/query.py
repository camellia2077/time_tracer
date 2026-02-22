# test/cases/query.py
from ..core.base import BaseTester, TestCounter
from ..conf.definitions import TestContext, TestReport, Colors


class QueryTester(BaseTester):
    def __init__(
        self,
        counter: TestCounter,
        module_order: int,
        context: TestContext,
        generated_db_file_name: str,
        daily_query_dates: list,
        monthly_query_months: list,
        weekly_query_weeks: list,
        yearly_query_years: list,
        recent_query_days: list,
        test_formats: list,
    ):
        super().__init__(counter, module_order, "query", context)
        self.db_file = (
            context.db_path
            if context.db_path
            else context.exe_path.parent / generated_db_file_name
        )
        self.daily_dates = daily_query_dates
        self.monthly_months = monthly_query_months
        self.weekly_weeks = weekly_query_weeks
        self.yearly_years = yearly_query_years
        self.recent_days = recent_query_days
        self.formats = test_formats

    def run_tests(self) -> TestReport:
        report = TestReport(module_name=self.module_name)

        if not self.db_file.exists():
            print(f"{Colors.RED}错误: 数据库文件不存在: {self.db_file}{Colors.RESET}")
            # 可以添加一个失败的 Result
            return report

        db_arg = ["--database", str(self.db_file)]

        # ... (构建 commands 逻辑不变) ...
        tests_to_run = []
        for date in self.daily_dates:
            for fmt in self.formats:
                tests_to_run.append(
                    (
                        f"Query Daily ({date}) [{fmt}]",
                        ["query", "day", date, "--format", fmt] + db_arg,
                    )
                )
        for month in self.monthly_months:
            for fmt in self.formats:
                tests_to_run.append(
                    (
                        f"Query Monthly ({month}) [{fmt}]",
                        ["query", "month", month, "--format", fmt] + db_arg,
                    )
                )
        for week in self.weekly_weeks:
            for fmt in self.formats:
                tests_to_run.append(
                    (
                        f"Query Weekly ({week}) [{fmt}]",
                        ["query", "week", week, "--format", fmt] + db_arg,
                    )
                )
        for year in self.yearly_years:
            for fmt in self.formats:
                tests_to_run.append(
                    (
                        f"Query Yearly ({year}) [{fmt}]",
                        ["query", "year", year, "--format", fmt] + db_arg,
                    )
                )
        for days in self.recent_days:
            for fmt in self.formats:
                tests_to_run.append(
                    (
                        f"Query Recent ({days}) [{fmt}]",
                        ["query", "recent", str(days), "--format", fmt] + db_arg,
                    )
                )

        for name, args in tests_to_run:
            res = self.run_command_test(name, args)
            report.results.append(res)

        return report
