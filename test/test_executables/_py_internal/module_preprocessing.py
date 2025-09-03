# module_preprocessing.py
from pathlib import Path
from .base_module import BaseTester, TestCounter
from typing import Optional

class PreprocessingTester(BaseTester):
    """Module for file pre-processing tests."""
    def __init__(self, counter: TestCounter, module_order: int,
                 executable_to_run: str, source_data_path: Path, converted_text_dir_name: str,
                 output_dir: Path,
                 specific_validation_path: Optional[str] = None):
        super().__init__(counter, module_order, "preprocessing",
                         executable_to_run, source_data_path, converted_text_dir_name,
                         output_dir)
        self.specific_validation_path = specific_validation_path

    def run_tests(self) -> bool:
        """Runs all pre-processing related tests using the new CLI commands."""
        source_path_str = str(self.source_data_path)
        
        # 定义所有需要运行的测试
        tests_to_run = [
            {
                "name": "1. Validate Source Files",
                "args": ["validate-source", source_path_str],
                "add_output": False  # 验证命令不需要输出目录
            },
            {
                "name": "2. Convert Source to JSON",
                "args": ["convert", source_path_str],
                "add_output": True   # 转换命令需要输出目录
            },
            {
                "name": "3. Validate Generated JSON Output (no day check)",
                "args": ["validate-output", str(self.processed_data_path)],
                "add_output": False  # 验证命令不需要输出目录
            }
        ]
        
        # 如果提供了特定路径，则添加带日期检查的验证测试
        if self.specific_validation_path:
            tests_to_run.append({
                "name": "4. Validate Specific JSON Output (with day check)",
                "args": ["validate-output", self.specific_validation_path, "--enable-day-check"],
                "add_output": False # 验证命令不需要输出目录
            })
        
        # 循环执行测试
        for test in tests_to_run:
            if not self.run_command_test(test["name"], test["args"], add_output_dir=test["add_output"]):
                return False
        
        return True