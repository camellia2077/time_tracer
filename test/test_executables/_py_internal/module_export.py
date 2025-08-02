# module_export.py
import shutil
from pathlib import Path
from .base_module import BaseTester, TestCounter

class ExportTester(BaseTester):
    """Module for data export tests."""
    def __init__(self, counter: TestCounter, module_order: int,
                 generated_db_file_name: str, 
                 is_bulk_mode: bool,         # 新增: 模式开关
                 specific_date: str,       # 新增: 指定日期
                 specific_month: str,      # 新增: 指定月份
                 period_export_days: str,
                 executable_to_run: str, source_data_path: Path, converted_text_dir_name: str):
        super().__init__(counter, module_order, "export",
                         executable_to_run, source_data_path, converted_text_dir_name)
        self.db_file = Path.cwd() / generated_db_file_name
        self.is_bulk_mode = is_bulk_mode
        self.specific_date = specific_date
        self.specific_month = specific_month
        self.period_days_to_export = period_export_days

    def run_tests(self) -> bool:
        """Runs all data export related tests."""
        if not self.db_file.exists():
            print(f"Warning: Skipping export tests because the database file '{self.db_file.name}' does not exist.")
            return True

        formats = ["md", "tex", "typ"]
        tests_to_run = []

        # 根据模式动态生成测试用例
        if self.is_bulk_mode:
            print("  模式: 全部导出")
            daily_cmd = ["--export", "all-daily"]
            monthly_cmd = ["--export", "all-monthly"]
            daily_test_name = "Bulk Export All Daily"
            monthly_test_name = "Bulk Export All Monthly"
        else:
            print("  模式: 指定日期/月份导出")
            daily_cmd = ["--export", "daily", self.specific_date]
            monthly_cmd = ["--export", "monthly", self.specific_month]
            daily_test_name = f"Specific Export Daily ({self.specific_date})"
            monthly_test_name = f"Specific Export Monthly ({self.specific_month})"

        # 为所有格式添加日报和月报的测试
        for fmt in formats:
            tests_to_run.append((f"{daily_test_name} [{fmt.upper()}]", daily_cmd + ["--format", fmt]))
            tests_to_run.append((f"{monthly_test_name} [{fmt.upper()}]", monthly_cmd + ["--format", fmt]))

        # 无论何种模式，都添加周期报告的测试
        period_cmd = ["--export", "period", self.period_days_to_export]
        for fmt in formats:
            tests_to_run.append((f"Period Export ({self.period_days_to_export}) [{fmt.upper()}]", period_cmd + ["--format", fmt]))
        
        # 执行所有生成的测试用例
        for name, args in tests_to_run:
            if not self.run_command_test(name, args):
                return False
                
        return True