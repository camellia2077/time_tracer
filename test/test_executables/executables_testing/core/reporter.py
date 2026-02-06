# test/core/reporter.py
from typing import List
from pathlib import Path
from ..conf.definitions import Colors, TestReport, SingleTestResult

class Reporter:
    """负责将测试报告以人类可读的方式打印到控制台"""

    def print_module_start(self, module_order: int, module_name: str):
        print(f"\n{Colors.CYAN}--- {module_order}. Running {module_name} Tasks ---{Colors.RESET}", flush=True)

    def print_module_report(self, report: TestReport):
        """打印单个模块的测试结果详情"""
        for res in report.results:
            self._print_single_result(res)
        
        if report.failed_count > 0:
            print(f"\n{Colors.RED}注意: 模块 '{report.module_name}' 有 {report.failed_count} 个失败项。{Colors.RESET}", flush=True)

    def _print_single_result(self, res: SingleTestResult):
        duration = res.execution_result.duration if res.execution_result else 0.0
        status_color = Colors.GREEN if res.status == "PASS" else Colors.RED
        
        # 1. 打印标题行
        cmd_name = "unknown"
        if res.execution_result and res.execution_result.command:
             cmd_name = Path(res.execution_result.command[0]).name
        
        print(f" -> {res.name} ({cmd_name}) ... {status_color}{res.status}{Colors.RESET} ({duration:.2f}s)", flush=True)

        # 2. 输出 messages (PASS/FAIL 均显示)
        if res.messages:
            for msg in res.messages:
                print(f"    {msg}", flush=True)

        # 3. 如果失败
        if res.status == "FAIL" and res.execution_result:
            print(f"    {Colors.RED}Error Details:{Colors.RESET}", flush=True)
            stderr_preview = res.execution_result.stderr.strip().splitlines()[:10]
            for line in stderr_preview:
                 print(f"    {Colors.RED}|{Colors.RESET} {line}", flush=True)

    def print_summary(self, all_reports: List[TestReport], total_duration: float, log_dir: Path):
        total_tests = sum(len(r.results) for r in all_reports)
        total_failed = sum(r.failed_count for r in all_reports)
        
        # 尝试寻找版本信息及大小信息用于摘要展示
        version_info = ""
        size_messages = []
        for report in all_reports:
            if report.module_name == "version_check":
                for res in report.results:
                    if res.name == "Application Version Check" and res.status == "PASS":
                        # 提取版本号
                        stdout = res.execution_result.stdout
                        if stdout:
                            lines = stdout.strip().splitlines()
                            version_info = lines[0] if lines else "Unknown"
                        # 提取大小信息 (messages)
                        size_messages = res.messages

        print("\n" + "="*25 + " TEST SUMMARY " + "="*25, flush=True)
        if version_info:
            print(f" Target Version  : {version_info}", flush=True)
        
        if size_messages:
            for msg in size_messages:
                # 过滤掉原本就在 messages 里的 Version Output 避免重复显示在摘要
                if "[Version Output]" not in msg:
                    print(f" {msg}", flush=True)

        print("-" * 64, flush=True)
        if total_failed == 0:
            print(f"{Colors.GREEN}[SUCCESS] All {total_tests} test steps completed successfully!{Colors.RESET}", flush=True)
        else:
            print(f"{Colors.RED}[FAILED] {total_failed}/{total_tests} test steps failed.{Colors.RESET}", flush=True)
            print(f"Check logs in: {log_dir}", flush=True)
            
        print(f"{Colors.CYAN}Total execution time: {total_duration:.2f} seconds.{Colors.RESET}", flush=True)
        print("="*64, flush=True)
