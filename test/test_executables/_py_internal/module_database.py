# module_database.py
from pathlib import Path
from .base_module import BaseTester, TestCounter

class DatabaseImportTester(BaseTester):
    """Module for database import tests."""
    def __init__(self, counter: TestCounter, module_order: int,
                 executable_to_run: str, source_data_path: Path, converted_text_dir_name: str):
        super().__init__(counter, module_order, "db_import",
                         executable_to_run, source_data_path, converted_text_dir_name)

    def run_tests(self) -> bool:
        """Runs all database import related tests."""
        if not self.processed_data_path.exists():
            print(f"Warning: Skipping DB import tests because the input directory '{self.processed_data_path.name}' does not exist.")
            return True # 跳过测试视为成功

        # 直接返回子测试的结果
        return self.run_command_test("Database Import Test (-p)", ["-p", str(self.processed_data_path)], stdin_input="y\n")