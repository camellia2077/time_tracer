# module_preprocessing.py
from pathlib import Path
from .base_module import BaseTester, TestCounter
from typing import Optional

class PreprocessingTester(BaseTester):
    """Module for file pre-processing tests."""
    def __init__(self, counter: TestCounter, module_order: int,
                 executable_to_run: str, source_data_path: Path, converted_text_dir_name: str,
                 output_dir: Path, # <--- 1. 在这里添加
                 specific_validation_path: Optional[str] = None):
        super().__init__(counter, module_order, "preprocessing",
                         executable_to_run, source_data_path, converted_text_dir_name,
                         output_dir) # <--- 2. 传递给父类
        self.specific_validation_path = specific_validation_path

    def run_tests(self) -> bool:
        """Runs all pre-processing related tests using the new CLI commands."""
        source_path_str = str(self.source_data_path)
        
        # [核心修改] 更新测试列表以匹配新的、独立的CLI命令
        # =================================================================
        tests_to_run = [
            ("1. Validate Source Files", 
             ["validate-source", source_path_str]),
             
            ("2. Convert Source to JSON", 
             ["convert", source_path_str]),
             
            # 注意: 这个测试验证的是转换步骤生成的输出
            # processed_data_path 是在 BaseTester 中根据命名规则自动计算出来的
            ("3. Validate Generated JSON Output (no day check)", 
             ["validate-output", str(self.processed_data_path)]),
        ]
        
        # 只有当 specific_validation_path 被提供时，才添加这个特殊的、带日期检查的验证测试
        if self.specific_validation_path:
            tests_to_run.append(
                ("4. Validate Specific JSON Output (with day check)", 
                 ["validate-output", self.specific_validation_path, "--enable-day-check"])
            )
        # =================================================================
        
        for name, args in tests_to_run:
            if not self.run_command_test(name, args):
                return False
        
        return True