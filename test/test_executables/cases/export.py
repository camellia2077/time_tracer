# cases/export.py
from pathlib import Path
from core.base import BaseTester, TestCounter
from conf.definitions import Colors, TestContext

class ExportTester(BaseTester):
    def __init__(self, counter: TestCounter, module_order: int, context: TestContext,
                 generated_db_file_name: str, 
                 is_bulk_mode: bool,
                 specific_dates: list,
                 specific_months: list,
                 period_export_days: list,
                 test_formats: list,
                 export_output_path: Path):
                 
        # 1. 将 context 传递给父类 BaseTester 
        super().__init__(counter, module_order, "export", context)
        
        # 2. 从 context 或传入参数中提取属性
        # 数据库路径优先使用 context.db_path，如果未定义则推导
        self.db_file = context.db_path if context.db_path else context.exe_path.parent / generated_db_file_name
        
        self.export_output_path = export_output_path
        self.is_bulk_mode = is_bulk_mode
        self.specific_dates = specific_dates
        self.specific_months = specific_months
        self.period_days_to_export = period_export_days
        self.formats = test_formats

    def run_tests(self) -> bool:
        if not self.db_file.exists():
            print(f"{Colors.RED}错误: 数据库文件不存在: {self.db_file}{Colors.RESET}")
            return False

        common_args = [
            "--database", str(self.db_file),
            "--output", str(self.export_output_path)
        ]

        formats = self.formats
        tests_to_run = []

        if self.is_bulk_mode:
            daily_cmd = ["export", "all-daily"]
            monthly_cmd = ["export", "all-monthly"]
            for fmt in formats:
                tests_to_run.append((f"Bulk Export All Daily [{fmt.upper()}]", daily_cmd + ["--format", fmt] + common_args))
                tests_to_run.append((f"Bulk Export All Monthly [{fmt.upper()}]", monthly_cmd + ["--format", fmt] + common_args))
        else:
            for date in self.specific_dates:
                daily_cmd = ["export", "daily", date]
                for fmt in formats:
                    tests_to_run.append((f"Specific Export Daily ({date}) [{fmt.upper()}]", daily_cmd + ["--format", fmt] + common_args))

            for month in self.specific_months:
                monthly_cmd = ["export", "monthly", month]
                for fmt in formats:
                    tests_to_run.append((f"Specific Export Monthly ({month}) [{fmt.upper()}]", monthly_cmd + ["--format", fmt] + common_args))

        period_days_str = ",".join(map(str, self.period_days_to_export))
        if self.is_bulk_mode:
            period_cmd = ["export", "all-period", period_days_str]
            for fmt in formats:
                tests_to_run.append((f"Bulk Period Export ({period_days_str}) [{fmt.upper()}]", period_cmd + ["--format", fmt] + common_args))
        else:
            for days in self.period_days_to_export:
                period_cmd = ["export", "period", str(days)]
                for fmt in formats:
                    tests_to_run.append((f"Specific Period Export ({days} days) [{fmt.upper()}]", period_cmd + ["--format", fmt] + common_args))
        
        all_passed = True
        for name, args in tests_to_run:
            # add_output_dir=False 避免基类重复添加默认输出路径
            if not self.run_command_test(name, args, add_output_dir=False):
                all_passed = False # 记录失败但继续执行后续指令以保存日志
                
        return all_passed