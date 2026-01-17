# test/cases/query.py
from pathlib import Path
from core.base import BaseTester, TestCounter
from conf.definitions import Colors, TestContext

class QueryTester(BaseTester):
    def __init__(self, counter: TestCounter, module_order: int, context: TestContext,
                 generated_db_file_name: str,
                 daily_query_dates: list,
                 monthly_query_months: list,
                 period_query_days: list,
                 test_formats: list):
                 
        super().__init__(counter, module_order, "query", context)

        # 数据库路径推导
        self.db_file = context.db_path if context.db_path else context.exe_path.parent / generated_db_file_name
        self.daily_dates = daily_query_dates
        self.monthly_months = monthly_query_months
        self.period_days = period_query_days
        self.formats = test_formats

    def run_tests(self) -> bool:
        """执行查询模块的测试用例 [新增]"""
        if not self.db_file.exists():
            print(f"{Colors.RED}错误: 数据库文件不存在: {self.db_file}{Colors.RESET}")
            return False

        db_arg = ["--database", str(self.db_file)]
        tests_to_run = []

        # 1. 构建每日查询指令
        for date in self.daily_dates:
            for fmt in self.formats:
                tests_to_run.append((
                    f"Query Daily ({date}) [{fmt.upper()}]", 
                    ["query", "daily", date, "--format", fmt] + db_arg
                ))
        
        # 2. 构建每月查询指令
        for month in self.monthly_months:
            for fmt in self.formats:
                tests_to_run.append((
                    f"Query Monthly ({month}) [{fmt.upper()}]", 
                    ["query", "monthly", month, "--format", fmt] + db_arg
                ))

        # 3. 构建周期查询指令
        for days in self.period_days:
            for fmt in self.formats:
                tests_to_run.append((
                    f"Query Period ({days} days) [{fmt.upper()}]", 
                    ["query", "period", str(days), "--format", fmt] + db_arg
                ))

        all_passed = True
        for name, args in tests_to_run:
            # 调用基类方法执行具体指令
            if not self.run_command_test(name, args):
                all_passed = False 
        
        return all_passed