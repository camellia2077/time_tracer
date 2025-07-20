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

    def run_tests(self):
        """Runs all data query related tests."""
        # This line was removed
        if not self.db_file.exists():
            print(f"Warning: Skipping query tests because the database file '{self.db_file.name}' does not exist.")
            return

        self.run_command_test("Data Query Test (-q d)", ["-q", "d", self.daily_date, "-f", "md"])
        self.run_command_test("Data Query Test (-q p)", ["-q", "p", self.period_days, "-f", "md"])
        self.run_command_test("Data Query Test (-q m)", ["-q", "m", self.monthly_month, "-f", "md"])