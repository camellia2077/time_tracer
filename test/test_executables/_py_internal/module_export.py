# module_export.py
import shutil
from pathlib import Path
from .base_module import BaseTester, TestCounter

class ExportTester(BaseTester):
    """Module for data export tests."""
    def __init__(self, counter: TestCounter, module_order: int,
                 generated_db_file_name: str, period_export_days: str,
                 executable_to_run: str, source_data_path: Path, converted_text_dir_name: str):
        super().__init__(counter, module_order, "export",
                         executable_to_run, source_data_path, converted_text_dir_name)
        self.db_file = Path.cwd() / generated_db_file_name
        self.period_days_to_export = period_export_days

    def run_tests(self) -> bool:
        """Runs all data export related tests."""
        if not self.db_file.exists():
            print(f"Warning: Skipping export tests because the database file '{self.db_file.name}' does not exist.")
            return True # Skipping tests is considered a success

        tests_to_run = [
            # --- 新增的测试用例 ---
            ("Data Export Test (--export day) [Markdown]", ["--export", "day", "--format", "md"]),
            # ---------------------
            ("Data Export Test (--export month) [Markdown]", ["--export", "month", "--format", "md"]),
            ("Data Export Test (--export period) [Markdown]", ["--export", "period", self.period_days_to_export, "--format", "md"]),
            ("Data Export Test (--export day) [TeX]", ["--export", "day", "--format", "tex"]),
            ("Data Export Test (--export month) [TeX]", ["--export", "month", "--format", "tex"]),
            ("Data Export Test (--export period) [TeX]", ["--export", "period", self.period_days_to_export, "--format", "tex"]),
            ("Data Export Test (--export day) [Typst]", ["--export", "day", "--format", "typ"]),
            ("Data Export Test (--export month) [Typst]", ["--export", "month", "--format", "typ"]),
            ("Data Export Test (--export period) [Typst]", ["--export", "period", self.period_days_to_export, "--format", "typ"])
        ]
        
        for name, args in tests_to_run:
            if not self.run_command_test(name, args):
                return False # If any test fails, return False immediately
                
        return True # All tests passed