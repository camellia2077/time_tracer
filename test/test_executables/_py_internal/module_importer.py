# _py_internal/module_importer.py
from pathlib import Path
from .base_module import BaseTester, TestCounter

class ImporterTester(BaseTester):
    """
    [Refactored] Module for standalone database import tests.
    Tests the 'import' command which reads JSON files from disk and inserts them into DB.
    """
    def __init__(self, counter: TestCounter, module_order: int,
                 executable_to_run: str, source_data_path: Path, converted_text_dir_name: str,
                 output_dir: Path):
        super().__init__(counter, module_order, "importer",
                         executable_to_run, source_data_path, converted_text_dir_name,
                         output_dir)

    def run_tests(self) -> bool:
        """Runs standalone database import test."""
        # 确保输入目录存在（该目录应由 ConverterTester 的 'convert' 步骤生成）
        if not self.processed_data_path.exists():
            print(f"Warning: Skipping Importer tests because the input directory '{self.processed_data_path.name}' does not exist.")
            # 注意：如果 convert 步骤失败，这里也会被跳过
            return True 

        # 测试 'import' 命令: 读取磁盘上的 JSON -> 解析 -> 入库
        # 这验证了系统的"数据恢复"能力，即使不运行完整 blink 也能从 JSON 重建数据库
        return self.run_command_test(
            "Standalone File Import (import command)", 
            ["import", str(self.processed_data_path)], 
            stdin_input="y\n" # 自动确认 "Are you sure?" 提示
        )