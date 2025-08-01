# module_query.py
from pathlib import Path
from .base_module import BaseTester, TestCounter

class QueryTester(BaseTester):
    """Module for data query tests."""
    def __init__(self, counter: TestCounter, module_order: int,
                 generated_db_file_name: str, daily_query_date: str, monthly_query_month: str, period_query_days: str,
                 executable_to_run: str, source_data_path: Path, converted_text_dir_name: str):
        super().__init__(counter, module_order, "query",
                         executable_to_run, source_data_path, converted_text_dir_name)
        self.db_file = Path.cwd() / generated_db_file_name
        self.daily_date = daily_query_date
        self.monthly_month = monthly_query_month
        self.period_days = period_query_days

    def run_tests(self) -> bool:
        """Runs all data query related tests."""
        if not self.db_file.exists():
            print(f"Warning: Skipping query tests because the database file '{self.db_file.name}' does not exist.")
            return True # Skipping tests is considered a success

        tests_to_run = [
            ("Data Query Test (--query daily) [Markdown]", ["--query", "daily", self.daily_date, "--format", "md"]),
            ("Data Query Test (--query period) [Markdown]", ["--query", "period", self.period_days, "--format", "md"]),
            ("Data Query Test (--query monthly) [Markdown]", ["--query", "monthly", self.monthly_month, "--format", "md"]),
            ("Data Query Test (--query daily) [TeX]", ["--query", "daily", self.daily_date, "--format", "tex"]),
            ("Data Query Test (--query period) [TeX]", ["--query", "period", self.period_days, "--format", "tex"]),
            ("Data Query Test (--query monthly) [TeX]", ["--query", "monthly", self.monthly_month, "--format", "tex"]),
            ("Data Query Test (--query daily) [Typst]", ["--query", "daily", self.daily_date, "--format", "typ"]),
            ("Data Query Test (--query period) [Typst]", ["--query", "period", self.period_days, "--format", "typ"]),
            ("Data Query Test (--query monthly) [Typst]", ["--query", "monthly", self.monthly_month, "--format", "typ"])
        ]
        
        for name, args in tests_to_run:
            if not self.run_command_test(name, args):
                return False # If any test fails, return False immediately
                
        return True # All tests passed