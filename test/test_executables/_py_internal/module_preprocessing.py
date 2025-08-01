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
        tests_to_run = [
            ("Pre-processing Test (--convert)", ["--convert", source_path_str]),
            ("Pre-processing Test (--validate-source)", ["--validate-source", source_path_str]),
            ("Pre-processing Test (--convert --validate-output)", ["--convert", "--validate-output", source_path_str]),
            ("Pre-processing Test (--convert --validate-output --enable-day-check)", ["--convert", "--validate-output", "--enable-day-check", source_path_str])
        ]
        
        for name, args in tests_to_run:
            if not self.run_command_test(name, args):
                return False  # If any test fails, return False immediately
        
        return True # All tests passed