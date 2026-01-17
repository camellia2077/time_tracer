import subprocess
import time
import re
from dataclasses import dataclass

@dataclass
class ExecutionResult:
    command: list
    return_code: int
    stdout: str
    stderr: str
    duration: float
    error: str = None

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
            # 纯粹的执行逻辑
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