# _py_internal/module_converter.py
from pathlib import Path
from .base_module import BaseTester, TestCounter
from typing import Optional

class IngestTester(BaseTester):
    """
    [Modified] Module for Blink tests.
    Previously handled validate/convert/validate-output, now streamlined to only run 'blink'.
    """
    def __init__(self, counter: TestCounter, module_order: int,
                 executable_to_run: str, source_data_path: Path, converted_text_dir_name: str,
                 output_dir: Path,
                 specific_validation_path: Optional[str] = None):
        super().__init__(counter, module_order, "ingest",
                         executable_to_run, source_data_path, converted_text_dir_name,
                         output_dir)
        self.specific_validation_path = specific_validation_path
    def run_tests(self) -> bool:
        """Runs the ingest command test."""
        source_path_str = str(self.source_data_path)
        
        tests_to_run = [
            {
                "name": "Full Pipeline (Blink)",
                "args": ["ingest", source_path_str],
                "add_output": True  # ingest 会产生输出文件，保留此选项
            }
        ]
        
        for test in tests_to_run:
            if not self.run_command_test(test["name"], test["args"], add_output_dir=test["add_output"]):
                return False
        
        return True