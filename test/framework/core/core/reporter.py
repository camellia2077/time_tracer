# test/core/reporter.py
import re
from typing import Any, Dict, List

from ..conf.definitions import Colors, SingleTestResult, TestReport


class Reporter:
    def __init__(self, no_color: bool = False, concise: bool = True):
        self.no_color = no_color
        self.concise = concise

    def print_module_start(self, module_order: int, module_name: str):
        print(self._colorize(
            f"\n--- {module_order}. Running {module_name} tasks ---",
            "CYAN",
        ), flush=True)

    def print_module_report(self, report: TestReport):
        print(
            f"Module '{report.module_name}': "
            f"pass={report.passed_count}, fail={report.failed_count} "
            f"({report.duration:.2f}s)",
            flush=True,
        )

        # 用户偏好：默认需要看到每条测试命令的 PASS/FAIL 明细行。
        # 不要把默认输出改成“仅显示模块成功/失败数量”。
        if self.concise:
            for result in report.results:
                if result.status == "FAIL":
                    self._print_failure_detail(report.module_name, result)
            return

        for result in report.results:
            self._print_single_result(report.module_name, result)

    def build_summary(self, all_reports: List[TestReport], total_duration: float,
                      log_dir) -> Dict[str, Any]:
        total_tests = sum(len(item.results) for item in all_reports)
        total_failed = sum(item.failed_count for item in all_reports)
        modules: List[Dict[str, Any]] = []
        failed_cases: List[Dict[str, Any]] = []

        for report in all_reports:
            modules.append({
                "name": report.module_name,
                "total": len(report.results),
                "passed": report.passed_count,
                "failed": report.failed_count,
            })
            for result in report.results:
                if result.status != "FAIL":
                    continue
                failed_cases.append(self._build_failed_case(report.module_name,
                                                           result))

        return {
            "success": total_failed == 0,
            "exit_code": 0 if total_failed == 0 else 1,
            "total_tests": total_tests,
            "total_failed": total_failed,
            "duration_seconds": round(total_duration, 3),
            "log_dir": str(log_dir),
            "modules": modules,
            "failed_cases": failed_cases,
            "error_message": "",
        }

    def print_summary(self, summary: Dict[str, Any]):
        print("\n" + "=" * 24 + " TEST SUMMARY " + "=" * 24, flush=True)
        print(
            f"Total: {summary['total_tests']}, "
            f"Failed: {summary['total_failed']}, "
            f"Duration: {summary['duration_seconds']:.3f}s",
            flush=True,
        )
        if summary["success"]:
            print(self._colorize("[SUCCESS] All test steps passed.", "GREEN"),
                  flush=True)
        else:
            print(self._colorize(
                f"[FAILED] {summary['total_failed']} test step(s) failed.",
                "RED",
            ), flush=True)
            print(f"Log directory: {summary['log_dir']}", flush=True)
        print("=" * 64, flush=True)

    def _print_single_result(self, module_name: str, result: SingleTestResult):
        duration = result.execution_result.duration if result.execution_result else 0.0
        status_color = "GREEN" if result.status == "PASS" else "RED"
        print(
            f" -> [{module_name}] {result.name} ... "
            f"{self._colorize(result.status, status_color)} ({duration:.2f}s)",
            flush=True,
        )
        for message in result.messages:
            print(f"    {self._strip_ansi(message)}", flush=True)
        if result.status == "FAIL":
            self._print_failure_detail(module_name, result)

    def _print_failure_detail(self, module_name: str, result: SingleTestResult):
        print(self._colorize(
            f"  [FAIL] [{module_name}] {result.name}",
            "RED",
        ), flush=True)
        if result.execution_result:
            print(f"    exit_code: {result.execution_result.return_code}",
                  flush=True)
            print(f"    command: {' '.join(result.execution_result.command)}",
                  flush=True)
            stderr_tail = self._tail_lines(result.execution_result.stderr, 10)
            if stderr_tail:
                print("    stderr (tail):", flush=True)
                for line in stderr_tail:
                    print(f"      {self._strip_ansi(line)}", flush=True)
        if result.log_file:
            print(f"    log_file: {result.log_file}", flush=True)
        for message in result.messages:
            print(f"    note: {self._strip_ansi(message)}", flush=True)

    def _build_failed_case(self, module_name: str,
                           result: SingleTestResult) -> Dict[str, Any]:
        execution = result.execution_result
        return {
            "module": module_name,
            "name": result.name,
            "status": result.status,
            "log_file": result.log_file or "",
            "messages": [self._strip_ansi(item) for item in result.messages],
            "return_code": execution.return_code if execution else -1,
            "command": execution.command if execution else [],
            "stderr_tail": self._tail_lines(execution.stderr if execution else "",
                                            20),
            "stdout_tail": self._tail_lines(execution.stdout if execution else "",
                                            20),
        }

    def _colorize(self, text: str, color_name: str) -> str:
        if self.no_color:
            return text
        color_value = getattr(Colors, color_name)
        return f"{color_value}{text}{Colors.RESET}"

    @staticmethod
    def _tail_lines(text: str, max_lines: int) -> List[str]:
        if not text:
            return []
        lines = text.strip().splitlines()
        return lines[-max_lines:]

    @staticmethod
    def _strip_ansi(text: str) -> str:
        ansi_escape = re.compile(r"\x1b\[[0-9;]*m")
        return ansi_escape.sub("", text)
