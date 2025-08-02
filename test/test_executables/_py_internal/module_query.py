# _py_internal/module_query.py
from pathlib import Path
from .base_module import BaseTester, TestCounter

class QueryTester(BaseTester):
    """用于数据查询测试的模块。"""
    def __init__(self, counter: TestCounter, module_order: int,
                 generated_db_file_name: str, 
                 daily_query_dates: list,
                 monthly_query_months: list,
                 period_query_days: list,  # 修改: 接收列表
                 executable_to_run: str, source_data_path: Path, converted_text_dir_name: str):
        super().__init__(counter, module_order, "query",
                         executable_to_run, source_data_path, converted_text_dir_name)
        self.db_file = Path.cwd() / generated_db_file_name
        self.daily_query_dates = daily_query_dates
        self.monthly_query_months = monthly_query_months
        self.period_query_days = period_query_days

    def run_tests(self) -> bool:
        """运行所有与数据查询相关的测试。"""
        if not self.db_file.exists():
            print(f"警告: 跳过查询测试，因为数据库文件 '{self.db_file.name}' 不存在。")
            return True

        formats = ["md", "tex", "typ"]
        tests_to_run = []

        # 循环遍历日期列表生成测试
        for date in self.daily_query_dates:
            for fmt in formats:
                tests_to_run.append(
                    (f"Data Query Test (--query daily {date}) [{fmt.upper()}]", 
                     ["--query", "daily", date, "--format", fmt])
                )

        # 循环遍历月份列表生成测试
        for month in self.monthly_query_months:
            for fmt in formats:
                tests_to_run.append(
                    (f"Data Query Test (--query monthly {month}) [{fmt.upper()}]", 
                     ["--query", "monthly", month, "--format", fmt])
                )
        
        # 修改: 将整数列表转换为逗号分隔的字符串以用于命令行
        period_days_str = ",".join(map(str, self.period_query_days))
        for fmt in formats:
            tests_to_run.append(
                (f"Data Query Test (--query period) [{fmt.upper()}]", 
                 ["--query", "period", period_days_str, "--format", fmt])
            )

        for name, args in tests_to_run:
            if not self.run_command_test(name, args):
                return False
                
        return True
