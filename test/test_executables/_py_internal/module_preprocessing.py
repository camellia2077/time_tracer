# module_preprocessing.py
from pathlib import Path
from .base_module import BaseTester, TestCounter

class PreprocessingTester(BaseTester):
    """Module for file pre-processing tests."""
    def __init__(self, counter: TestCounter, module_order: int,
                 executable_to_run: str, source_data_path: Path, converted_text_dir_name: str):
        super().__init__(counter, module_order, "preprocessing",
                         executable_to_run, source_data_path, converted_text_dir_name)

    def run_tests(self) -> bool:
        """Runs all pre-processing related tests."""
        source_path_str = str(self.source_data_path)
        
        # 修正了测试以匹配新的CLI用法：preprocess <path> [options...]
        tests_to_run = [
            ("Pre-processing Test (preprocess --convert)", 
             ["preprocess", source_path_str, "--convert"]),
             
            ("Pre-processing Test (preprocess --validate-source)", 
             ["preprocess", source_path_str, "--validate-source"]),
             
            ("Pre-processing Test (preprocess --convert --validate-output)", 
             ["preprocess", source_path_str, "--convert", "--validate-output"]),
             
            ("Pre-processing Test (preprocess --convert --validate-output --enable-day-check)", 
             ["preprocess", source_path_str, "--convert", "--validate-output", "--enable-day-check"])
        ]
        
        for name, args in tests_to_run:
            if not self.run_command_test(name, args):
                return False  # 如果任何测试失败，立即返回 False
        
        return True # 所有测试都通过了