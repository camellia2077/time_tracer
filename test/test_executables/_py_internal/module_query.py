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
            return True # 跳过测试视为成功

        tests_to_run = [
            ("Data Query Test (-q d) [Markdown]", ["-q", "d", self.daily_date, "-f", "md"]),
            ("Data Query Test (-q p) [Markdown]", ["-q", "p", self.period_days, "-f", "md"]),
            ("Data Query Test (-q m) [Markdown]", ["-q", "m", self.monthly_month, "-f", "md"]),
            ("Data Query Test (-q d) [TeX]", ["-q", "d", self.daily_date, "-f", "tex"]),
            ("Data Query Test (-q p) [TeX]", ["-q", "p", self.period_days, "-f", "tex"]),
            ("Data Query Test (-q m) [TeX]", ["-q", "m", self.monthly_month, "-f", "tex"])
        ]
        
        for name, args in tests_to_run:
            if not self.run_command_test(name, args):
                return False # 如果任何一个测试失败，立即返回 False
                
        return True # 所有测试都通过了