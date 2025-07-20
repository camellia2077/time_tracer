# module_query.py
from pathlib import Path
from .base_module import BaseTester, TestCounter

class QueryTester(BaseTester):
    """Module for data query tests."""
    # 构造函数保持不变，它正确地接收所有配置
    def __init__(self, counter: TestCounter, module_order: int, 
                 generated_db_file_name: str, daily_query_date: str, monthly_query_month: str, period_query_days: str,
                 executable_to_run: str, source_data_path: Path, converted_text_dir_name: str):
        super().__init__(counter, module_order, "query", 
                         executable_to_run, source_data_path, converted_text_dir_name)
        # 保存此模块特有的配置
        self.db_file = Path.cwd() / generated_db_file_name
        self.daily_date = daily_query_date
        self.monthly_month = monthly_query_month
        self.period_days = period_query_days

    def run_tests(self):
        """Runs all data query related tests."""
        self._log_to_console(f"\n--- [Module Start]: Data Query (Logs -> {self.reports_dir.relative_to(Path.cwd())}) ---\n")
        
        if not self.db_file.exists():
            self._log_to_console(f"Warning: Skipping query tests because the database file '{self.db_file.name}' does not exist.")
            return
            
        # [确认] 此处已正确使用 self.period_days (值为 "7")
        self.run_command_test("Data Query Test (-q d)", ["-q", "d", self.daily_date, "-f", "md"])
        self.run_command_test("Data Query Test (-q p)", ["-q", "p", self.period_days, "-f", "md"])
        self.run_command_test("Data Query Test (-q m)", ["-q", "m", self.monthly_month, "-f", "md"])