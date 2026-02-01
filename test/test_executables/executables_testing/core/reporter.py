# test/core/reporter.py
from typing import List
from pathlib import Path
from ..conf.definitions import Colors, TestReport, SingleTestResult

class Reporter:
    """负责将测试报告以人类可读的方式打印到控制台"""

    def print_module_start(self, module_order: int, module_name: str):
        print(f"\n{Colors.CYAN}--- {module_order}. Running {module_name} Tasks ---{Colors.RESET}")

    def print_module_report(self, report: TestReport):
        """打印单个模块的测试结果详情"""
        for res in report.results:
            self._print_single_result(res)
        
        if report.failed_count > 0:
            print(f"\n{Colors.RED}注意: 模块 '{report.module_name}' 有 {report.failed_count} 个失败项。{Colors.RESET}")

    def _print_single_result(self, res: SingleTestResult):
        duration = res.execution_result.duration if res.execution_result else 0.0
        status_color = Colors.GREEN if res.status == "PASS" else Colors.RED
        
        # 1. 打印标题行
        # 尝试从 command path 中提取 exe 名字，或者直接用 test name
        cmd_name = "unknown"
        if res.execution_result and res.execution_result.command:
             cmd_name = Path(res.execution_result.command[0]).name
        
        print(f" -> {res.name} ({cmd_name}) ... {status_color}{res.status}{Colors.RESET} ({duration:.2f}s)")

        # 2. 输出 messages (PASS/FAIL 均显示，方便定位断言失败)
        if res.messages:
            for msg in res.messages:
                print(f"    {msg}")

        # 3. 如果失败，打印错误信息
        if res.status == "FAIL" and res.execution_result:
            print(f"    {Colors.RED}Error Details:{Colors.RESET}")
            stderr_preview = res.execution_result.stderr.strip().splitlines()[:10]
            for line in stderr_preview:
                 print(f"    {Colors.RED}│{Colors.RESET} {line}")

    def print_summary(self, all_reports: List[TestReport], total_duration: float, log_dir: Path):
        total_tests = sum(len(r.results) for r in all_reports)
        total_failed = sum(r.failed_count for r in all_reports)
        
        print("\n" + "="*20 + " Test Summary " + "="*20)
        if total_failed == 0:
            print(f"{Colors.GREEN}✅ All {total_tests} test steps completed successfully!{Colors.RESET}")
        else:
            print(f"{Colors.RED}❌ {total_failed}/{total_tests} test steps failed.{Colors.RESET}")
            print(f"Check logs in: {log_dir}")
            
        print(f"{Colors.CYAN}Total execution time: {total_duration:.2f} seconds.{Colors.RESET}")
