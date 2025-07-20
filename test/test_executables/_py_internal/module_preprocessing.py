# module_preprocessing.py
from pathlib import Path
from .base_module import BaseTester, TestCounter

class PreprocessingTester(BaseTester):
    """Module for file pre-processing tests."""
    # [修改] 更新构造函数以接收并传递配置
    def __init__(self, counter: TestCounter, module_order: int, 
                 executable_to_run: str, source_data_path: Path, converted_text_dir_name: str):
        super().__init__(counter, module_order, "preprocessing", 
                         executable_to_run, source_data_path, converted_text_dir_name)

    def run_tests(self):
        """Runs all pre-processing related tests."""
        self._log_to_console(f"\n--- [Module Start]: Pre-processing (Logs -> {self.reports_dir.relative_to(Path.cwd())}) ---\n")
        source_path_str = str(self.source_data_path)
        self.run_command_test("Pre-processing Test (-c)", ["-c", source_path_str])
        self.run_command_test("Pre-processing Test (-vs)", ["-vs", source_path_str])
        self.run_command_test("Pre-processing Test (-c -vo)", ["-c", "-vo", source_path_str])
        self.run_command_test("Pre-processing Test (-c -vo -edc)", ["-c", "-vo", "-edc", source_path_str])