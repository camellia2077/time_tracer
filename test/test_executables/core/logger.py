from pathlib import Path
from conf.definitions import Colors
from core.executor import ExecutionResult, CommandExecutor

class TestLogger:
    """专注负责日志文件的写入和控制台输出"""
    
    def __init__(self, log_dir: Path):
        self.log_dir = log_dir
        if not self.log_dir.exists():
            self.log_dir.mkdir(parents=True, exist_ok=True)

    def log_result(self, test_name: str, log_filename: str, result: ExecutionResult, print_stdout: bool = False):
        log_path = self.log_dir / log_filename
        status = "OK" if result.return_code == 0 else "FAIL"
        
        # 1. 写文件
        with open(log_path, 'w', encoding='utf-8') as f:
            f.write(f"Test: {test_name}\n")
            f.write(f"Command: {' '.join(result.command)}\n")
            f.write(f"Exit Code: {result.return_code}\n")
            if result.error:
                f.write(f"Exception: {result.error}\n")
            f.write("\n--- STDOUT ---\n")
            f.write(CommandExecutor.strip_ansi_codes(result.stdout) or 'None')
            f.write("\n\n--- STDERR ---\n")
            f.write(CommandExecutor.strip_ansi_codes(result.stderr) or 'None')

        # 2. 控制台输出
        self._print_console(test_name, result, status, print_stdout)

    def _print_console(self, test_name: str, result: ExecutionResult, status: str, print_stdout: bool):
        # 提取原 base.py 中的打印逻辑
        cmd_name = Path(result.command[0]).name if result.command else "unknown"
        status_color = Colors.GREEN if status == "OK" else Colors.RED
        
        print(f" -> {cmd_name} ... {status_color}{status}{Colors.RESET} ({result.duration:.2f}s)")
        
        if print_stdout and status == "OK" and result.stdout:
            print(f"    {Colors.GREEN}├─ Output snippet:{Colors.RESET}")
            # 只打印前几行，避免刷屏
            for line in result.stdout.strip().splitlines()[:5]:
                print(f"    {Colors.GREEN}│{Colors.RESET} {line}")