# test/cases/ingest.py
from pathlib import Path
from typing import Optional
from ..core.base import BaseTester, TestCounter
from ..conf.definitions import TestContext

class IngestTester(BaseTester):
    def __init__(self, counter: TestCounter, module_order: int, context: TestContext,
                 specific_validation_path: Optional[str] = None):
        
        # 传递 context 给父类 BaseTester
        super().__init__(counter, module_order, "ingest", context)
        self.specific_validation_path = specific_validation_path
        
    def run_tests(self) -> bool:
        # 使用 self.ctx 获取路径
        source_path_str = str(self.ctx.source_data_path)
        tests_to_run = [
            {
                "name": "Full Pipeline (Blink)",
                "args": ["ingest", source_path_str],
                "add_output": True
            }
        ]
        for test in tests_to_run:
            if not self.run_command_test(test["name"], test["args"], add_output_dir=test["add_output"]):
                return False
        return True