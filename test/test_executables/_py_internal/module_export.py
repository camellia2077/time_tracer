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
        # [修改] 使用从 main.py 传入的 period_export_days ("7,30,90")，而不是硬编码的 "7"
        self.period_days_to_export = period_export_days

    def run_tests(self):
        """Runs all data export related tests."""
        self._log_to_console(f"\n--- [Module Start]: Data Export (Logs -> {self.reports_dir.relative_to(Path.cwd())}) ---\n")
        
        export_dir = Path.cwd() / "Export"
        if export_dir.exists():
            self._log_to_console(f"Warning: Deleting existing export directory: {export_dir}")
            try:
                shutil.rmtree(export_dir)
                self._log_to_console(f"    ✔️ Successfully deleted.")
            except OSError as e:
                self._log_to_console(f"    ❌ Error deleting directory {export_dir}: {e}")
                return

        if not self.db_file.exists():
            self._log_to_console(f"Warning: Skipping export tests because the database file '{self.db_file.name}' does not exist.")
            return

        self.run_command_test("Data Export Test (-e d)", ["-e", "d", "-f", "md"])
        self.run_command_test("Data Export Test (-e m)", ["-e", "m", "-f", "md"])
        # [修改] 使用正确的变量，它现在包含了 "7,30,90"
        self.run_command_test("Data Export Test (-e p)", ["-e", "p", self.period_days_to_export, "-f", "md"])

        self._log_to_console(f"\n  [File Check]:")
        if export_dir.exists() and export_dir.is_dir():
            self._log_to_console(f"    ✔️ Success! Output directory '{export_dir.name}' was created.")
        else:
            self._log_to_console(f"    ❌ Failure! Output directory '{export_dir.name}' was NOT created.")