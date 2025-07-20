# module_database.py
from pathlib import Path
from .base_module import BaseTester, TestCounter

class DatabaseImportTester(BaseTester):
    """Module for database import tests."""
    # [修改] 更新构造函数以接收并传递配置
    def __init__(self, counter: TestCounter, module_order: int, 
                 executable_to_run: str, source_data_path: Path, converted_text_dir_name: str):
        super().__init__(counter, module_order, "db_import", 
                         executable_to_run, source_data_path, converted_text_dir_name)

    def run_tests(self):
        """Runs all database import related tests."""
        self._log_to_console(f"\n--- [Module Start]: Database Import (Logs -> {self.reports_dir.relative_to(Path.cwd())}) ---\n")
        
        if not self.processed_data_path.exists():
            self._log_to_console(f"Warning: Skipping DB import tests because the input directory '{self.processed_data_path.name}' does not exist.")
            return
            
        self.run_command_test("Database Import Test (-p)", ["-p", str(self.processed_data_path)], stdin_input="y\n")