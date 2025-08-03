from pathlib import Path
from .base_module import BaseTester, TestCounter

class ExportTester(BaseTester):
    """用于数据导出测试的模块。"""
    def __init__(self, counter: TestCounter, module_order: int,
                 generated_db_file_name: str, 
                 is_bulk_mode: bool,
                 specific_dates: list,
                 specific_months: list,
                 period_export_days: list,
                 test_formats: list,
                 executable_to_run: str, source_data_path: Path, converted_text_dir_name: str):
        super().__init__(counter, module_order, "export",
                         executable_to_run, source_data_path, converted_text_dir_name)
        self.db_file = Path.cwd() / generated_db_file_name
        self.is_bulk_mode = is_bulk_mode
        self.specific_dates = specific_dates
        self.specific_months = specific_months
        self.period_days_to_export = period_export_days
        self.formats = test_formats

    def run_tests(self) -> bool:
        """运行所有与数据导出相关的测试。"""
        if not self.db_file.exists():
            print(f"警告: 跳过导出测试，因为数据库文件 '{self.db_file.name}' 不存在。")
            return True

        formats = self.formats
        tests_to_run = []

        if self.is_bulk_mode:
            print("  模式: 全部导出")
            # 修正: 将 "--export" 改为 "export"
            daily_cmd = ["export", "all-daily"]
            monthly_cmd = ["export", "all-monthly"]
            daily_test_name = "Bulk Export All Daily"
            monthly_test_name = "Bulk Export All Monthly"
            
            for fmt in formats:
                tests_to_run.append((f"{daily_test_name} [{fmt.upper()}]", daily_cmd + ["--format", fmt]))
                tests_to_run.append((f"{monthly_test_name} [{fmt.upper()}]", monthly_cmd + ["--format", fmt]))
        else:
            print("  模式: 指定日期/月份导出")
            for date in self.specific_dates:
                # 修正: 将 "--export" 改为 "export"
                daily_cmd = ["export", "daily", date]
                daily_test_name = f"Specific Export Daily ({date})"
                for fmt in formats:
                    tests_to_run.append((f"{daily_test_name} [{fmt.upper()}]", daily_cmd + ["--format", fmt]))

            for month in self.specific_months:
                # 修正: 将 "--export" 改为 "export"
                monthly_cmd = ["export", "monthly", month]
                monthly_test_name = f"Specific Export Monthly ({month})"
                for fmt in formats:
                    tests_to_run.append((f"{monthly_test_name} [{fmt.upper()}]", monthly_cmd + ["--format", fmt]))

        period_days_str = ",".join(map(str, self.period_days_to_export))
        
        if self.is_bulk_mode:
            # 修正: 将 "--export" 改为 "export"
            period_cmd = ["export", "all-period", period_days_str]
            period_test_name = f"Bulk Period Export ({period_days_str})"
            for fmt in formats:
                tests_to_run.append((f"{period_test_name} [{fmt.upper()}]", period_cmd + ["--format", fmt]))
        else:
            for days in self.period_days_to_export:
                # 修正: 将 "--export" 改为 "export"
                period_cmd = ["export", "period", str(days)]
                period_test_name = f"Specific Period Export ({days} days)"
                for fmt in formats:
                    tests_to_run.append((f"{period_test_name} [{fmt.upper()}]", period_cmd + ["--format", fmt]))
        
        for name, args in tests_to_run:
            if not self.run_command_test(name, args):
                return False
                
        return True