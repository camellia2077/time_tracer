# module_export.py
import shutil
from pathlib import Path
from .base_module import BaseTester, TestCounter
from . import config

class ExportTester(BaseTester):
    """Module for data export tests."""
    def __init__(self, counter: TestCounter, module_order: int):
        super().__init__(counter, module_order, "export")

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

        db_file = Path.cwd() / config.GENERATED_DB_FILE_NAME
        if not db_file.exists():
            self._log_to_console(f"Warning: Skipping export tests because the database file '{db_file.name}' does not exist.")
            return

        self.run_command_test("Data Export Test (-e d)", ["-e", "d"])
        self.run_command_test("Data Export Test (-e m)", ["-e", "m"])
        self.run_command_test("Data Export Test (-e p)", ["-e", "p", config.PERIOD_EXPORT_DAYS])

        self._log_to_console(f"\n  [File Check]:")
        if export_dir.exists() and export_dir.is_dir():
            self._log_to_console(f"    ✔️ Success! Output directory '{export_dir.name}' was created.")
        else:
            self._log_to_console(f"    ❌ Failure! Output directory '{export_dir.name}' was NOT created.")