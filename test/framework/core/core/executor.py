import re
import subprocess
import sys
import time

from ..conf.definitions import ExecutionResult


class CommandExecutor:
    def __init__(self, show_output: str = "none"):
        self.show_output = show_output if show_output in {
            "none", "fail", "all"
        } else "none"

    @staticmethod
    def strip_ansi_codes(text: str) -> str:
        if not text:
            return ""
        ansi_escape = re.compile(r"\x1b\[[0-9;]*m")
        return ansi_escape.sub("", text)

    def run(self, command: list, cwd=None,
            input_str: str = None) -> ExecutionResult:
        start_time = time.monotonic()
        try:
            result = subprocess.run(
                command,
                input=input_str,
                cwd=cwd,
                capture_output=True,
                text=True,
                check=False,
                encoding="utf-8",
                errors="ignore",
            )
            duration = time.monotonic() - start_time

            self._display_process_output(result.returncode, result.stdout,
                                         result.stderr)
            return ExecutionResult(
                command=command,
                return_code=result.returncode,
                stdout=result.stdout,
                stderr=result.stderr,
                duration=duration,
            )
        except Exception as error:
            return ExecutionResult(
                command=command,
                return_code=-1,
                stdout="",
                stderr=str(error),
                duration=time.monotonic() - start_time,
                error=str(error),
            )

    def _display_process_output(self, return_code: int, stdout_text: str,
                                stderr_text: str):
        should_show = (self.show_output == "all") or (
            self.show_output == "fail" and return_code != 0)
        if not should_show:
            return

        if stdout_text:
            print(stdout_text, end="", flush=True)
        if stderr_text:
            print(stderr_text, end="", file=sys.stderr, flush=True)
