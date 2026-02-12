# src/tt_testing/core/logger.py
from pathlib import Path

from ..conf.definitions import ExecutionResult
from .executor import CommandExecutor


class TestLogger:
    def __init__(self, log_dir: Path):
        self.log_dir = log_dir
        self.log_dir.mkdir(parents=True, exist_ok=True)

    def log_result(self, test_name: str, log_filename: str,
                   result: ExecutionResult) -> Path:
        log_path = self.log_dir / log_filename
        with open(log_path, "w", encoding="utf-8") as file:
            file.write(f"Test: {test_name}\n")
            file.write(f"Command: {' '.join(result.command)}\n")
            file.write(f"Exit Code: {result.return_code}\n")
            if result.error:
                file.write(f"Exception: {result.error}\n")
            file.write("\n--- STDOUT ---\n")
            file.write(CommandExecutor.strip_ansi_codes(result.stdout) or "None")
            file.write("\n\n--- STDERR ---\n")
            file.write(CommandExecutor.strip_ansi_codes(result.stderr) or "None")
        return log_path
