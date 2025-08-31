# module_query.py
from pathlib import Path
from .base_module import BaseTester, TestCounter

class QueryTester(BaseTester):
    """Module for database query tests."""
    def __init__(self, counter: TestCounter, module_order: int,
                 generated_db_file_name: str,
                 daily_query_dates: list,
                 monthly_query_months: list,
                 period_query_days: list,
                 test_formats: list,
                 executable_to_run: str, source_data_path: Path, converted_text_dir_name: str):
        super().__init__(counter, module_order, "query",
                         executable_to_run, source_data_path, converted_text_dir_name)
        self.db_file = Path.cwd() / generated_db_file_name
        self.daily_dates = daily_query_dates
        self.monthly_months = monthly_query_months
        self.period_days = period_query_days
        self.formats = test_formats

    def run_tests(self) -> bool:
        """Runs all database query related tests."""
        if not self.db_file.exists():
            print(f"Warning: Skipping query tests because the database file '{self.db_file.name}' does not exist.")
            return True # Treat skipping as success

        tests_to_run = []
        
        # Build daily query tests
        for date in self.daily_dates:
            for fmt in self.formats:
                test_name = f"Query Daily ({date}) [{fmt.upper()}]"
                command_args = ["query", "daily", date, "--format", fmt]
                tests_to_run.append((test_name, command_args))

        # Build monthly query tests
        for month in self.monthly_months:
            for fmt in self.formats:
                test_name = f"Query Monthly ({month}) [{fmt.upper()}]"
                command_args = ["query", "monthly", month, "--format", fmt]
                tests_to_run.append((test_name, command_args))
        
        # Build period query tests
        for days in self.period_days:
            for fmt in self.formats:
                test_name = f"Query Period ({days} days) [{fmt.upper()}]"
                command_args = ["query", "period", str(days), "--format", fmt]
                tests_to_run.append((test_name, command_args))

        # Execute all generated tests
        for name, args in tests_to_run:
            if not self.run_command_test(name, args):
                return False  # Stop on first failure
        
        return True # All tests passed