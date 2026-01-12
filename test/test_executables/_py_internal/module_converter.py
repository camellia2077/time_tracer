# _py_internal/module_converter.py
from pathlib import Path
from .base_module import BaseTester, TestCounter
from typing import Optional

class ConverterTester(BaseTester):
    """
    [Refactored] Module for converter pipeline tests.
    Formerly PreprocessingTester.
    """
    def __init__(self, counter: TestCounter, module_order: int,
                 executable_to_run: str, source_data_path: Path, converted_text_dir_name: str,
                 output_dir: Path,
                 specific_validation_path: Optional[str] = None):
        # [修改] 将内部名称从 "preprocessing" 改为 "converter"
        # 这会影响生成的日志文件夹名称 (e.g., "1_Converter")
        super().__init__(counter, module_order, "converter",
                         executable_to_run, source_data_path, converted_text_dir_name,
                         output_dir)
        self.specific_validation_path = specific_validation_path

    def run_tests(self) -> bool:
        """Runs all converter pipeline related tests using the new CLI commands."""
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
            # 注意：--enable-day-check 是旧参数吗？
            # 你的 C++ CliParser 现在使用的是 --date-check <mode>
            # 建议这里同步更新为新的 CLI 参数格式
            tests_to_run.append({
                "name": "4. Validate Specific JSON Output (with strict date check)",
                # [建议修改] 更新为新的参数格式
                "args": ["validate-output", self.specific_validation_path, "--date-check", "full"],
                "add_output": False
            })
        
        # 循环执行测试
        for test in tests_to_run:
            if not self.run_command_test(test["name"], test["args"], add_output_dir=test["add_output"]):
                return False
        
        return True