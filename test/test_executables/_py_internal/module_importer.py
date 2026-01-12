# _py_internal/module_importer.py
from pathlib import Path
from .base_module import BaseTester, TestCounter

class ImporterTester(BaseTester):
    """
    [Refactored] Module for database import tests.
    Formerly DatabaseImportTester (db_inserter).
    """
    def __init__(self, counter: TestCounter, module_order: int,
                 executable_to_run: str, source_data_path: Path, converted_text_dir_name: str,
                 output_dir: Path):
        # [修改] 将内部名称从 "db_import" 改为 "importer"
        super().__init__(counter, module_order, "importer",
                         executable_to_run, source_data_path, converted_text_dir_name,
                         output_dir)

    def run_tests(self) -> bool:
        """Runs all database import related tests."""
        if not self.processed_data_path.exists():
            print(f"Warning: Skipping Importer tests because the input directory '{self.processed_data_path.name}' does not exist.")
            return True # 跳过测试被视为成功

        # 使用 'import' 命令：import <path>
        # 这里的 CLI 命令名称已经是 "import" 了，所以只需要改类名和模块ID即可
        return self.run_command_test(
            "Database Import Test (import)", 
            ["import", str(self.processed_data_path)], 
            stdin_input="y\n" # 自动确认交互提示
        )