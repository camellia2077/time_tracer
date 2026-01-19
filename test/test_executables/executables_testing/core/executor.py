import subprocess
import time
import re
# [修改] 从 definitions 导入 ExecutionResult
from ..conf.definitions import ExecutionResult

class CommandExecutor:
    """专注负责子进程的调用与结果捕获"""
    
    @staticmethod
    def strip_ansi_codes(text: str) -> str:
        if not text: return ""
        ansi_escape = re.compile(r'\x1b\[[0-9;]*m')
        return ansi_escape.sub('', text)

    def run(self, command: list, cwd=None, input_str: str = None) -> ExecutionResult:
        start_time = time.monotonic()
        try:
            result = subprocess.run(
                command,
                input=input_str,
                cwd=cwd,
                capture_output=True,
                text=True,
                check=False,
                encoding='utf-8',
                errors='ignore'
            )
            duration = time.monotonic() - start_time
            
            return ExecutionResult(
                command=command,
                return_code=result.returncode,
                stdout=result.stdout,
                stderr=result.stderr,
                duration=duration
            )
        except Exception as e:
            return ExecutionResult(
                command=command,
                return_code=-1,
                stdout="",
                stderr=str(e),
                duration=time.monotonic() - start_time,
                error=str(e)
            )