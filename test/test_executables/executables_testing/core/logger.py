# src/tt_testing/core/logger.py
from pathlib import Path
from ..conf.definitions import Colors, ExecutionResult
from .executor import CommandExecutor

class TestLogger:
    """专注负责日志文件的写入，不再负责控制台输出"""
    
    def __init__(self, log_dir: Path):
        self.log_dir = log_dir
        # [Lazy Creation] 只有当 Logger 被实例化时才创建目录
        if not self.log_dir.exists():
            self.log_dir.mkdir(parents=True, exist_ok=True)

    def log_result(self, test_name: str, log_filename: str, result: ExecutionResult):
        log_path = self.log_dir / log_filename
        
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