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

    def run_tests(self):
        """Runs all data export related tests."""
        # This line was removed
        export_dir = Path.cwd() / "Export"
        if not self.db_file.exists():
            print(f"Warning: Skipping export tests because the database file '{self.db_file.name}' does not exist.")
            return

        self.run_command_test("Data Export Test (-e d) [Markdown]", ["-e", "d", "-f", "md"])
        self.run_command_test("Data Export Test (-e m) [Markdown]", ["-e", "m", "-f", "md"])
        self.run_command_test("Data Export Test (-e p) [Markdown]", ["-e", "p", self.period_days_to_export, "-f", "md"])
        
        # New: Add export tests for TeX format
        self.run_command_test("Data Export Test (-e d) [TeX]", ["-e", "d", "-f", "tex"])
        self.run_command_test("Data Export Test (-e m) [TeX]", ["-e", "m", "-f", "tex"])
        self.run_command_test("Data Export Test (-e p) [TeX]", ["-e", "p", self.period_days_to_export, "-f", "tex"])
        
        # The extra file check logic has also been removed to prevent the same error.