# module_preprocessing.py
from pathlib import Path
from .base_module import BaseTester, TestCounter

class PreprocessingTester(BaseTester):
    """Module for file pre-processing tests."""
    def __init__(self, counter: TestCounter, module_order: int,
                 executable_to_run: str, source_data_path: Path, converted_text_dir_name: str):
        super().__init__(counter, module_order, "preprocessing",
                         executable_to_run, source_data_path, converted_text_dir_name)

    def run_tests(self):
        """Runs all pre-processing related tests."""
        # This line was removed as it's now handled in main.py
        source_path_str = str(self.source_data_path)
        self.run_command_test("Pre-processing Test (-c)", ["-c", source_path_str])
        self.run_command_test("Pre-processing Test (-vs)", ["-vs", source_path_str])
        self.run_command_test("Pre-processing Test (-c -vo)", ["-c", "-vo", source_path_str])
        self.run_command_test("Pre-processing Test (-c -vo -edc)", ["-c", "-vo", "-edc", source_path_str])