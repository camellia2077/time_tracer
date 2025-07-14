# module_query.py
from pathlib import Path
from .base_module import BaseTester, TestCounter
from . import config

class QueryTester(BaseTester):
    """Module for data query tests."""
    def __init__(self, counter: TestCounter, module_order: int):
        super().__init__(counter, module_order, "query")

    def run_tests(self):
        """Runs all data query related tests."""
        self._log_to_console(f"\n--- [Module Start]: Data Query (Logs -> {self.reports_dir.relative_to(Path.cwd())}) ---\n")
        
        db_file = Path.cwd() / config.GENERATED_DB_FILE_NAME
        if not db_file.exists():
            self._log_to_console(f"Warning: Skipping query tests because the database file '{db_file.name}' does not exist.")
            return
            
        self.run_command_test("Data Query Test (-q d)", ["-q", "d", config.DAILY_QUERY_DATE])
        self.run_command_test("Data Query Test (-q p)", ["-q", "p", config.PERIOD_QUERY_DAYS])
        self.run_command_test("Data Query Test (-q m)", ["-q", "m", config.MONTHLY_QUERY_MONTH])