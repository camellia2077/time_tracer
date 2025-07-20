# base_module.py
import re
import subprocess
import time
from pathlib import Path

# --- ANSI Color Codes ---
class Colors:
    GREEN = '\033[92m'
    RED = '\033[91m'
    RESET = '\033[0m'

# This contains the foundational code shared by all other modules
def strip_ansi_codes(text: str) -> str:
    """Removes ANSI escape codes (for color) from text."""
    ansi_escape_pattern = re.compile(r'\x1b\[[0-9;]*m')
    return ansi_escape_pattern.sub('', text)

class TestCounter:
    """A simple counter shared across test modules."""
    def __init__(self):
        self.value = 0
    def increment(self):
        self.value += 1
        return self.value

class BaseTester:
    """Base class for all test modules."""
    def __init__(self, counter: TestCounter, module_order: int, reports_sub_dir_name: str,
                 executable_to_run: str, source_data_path: Path, converted_text_dir_name: str):
        self.executable_path = Path.cwd() / executable_to_run
        self.source_data_path = source_data_path
        self.processed_data_path = Path.cwd() / converted_text_dir_name
        self.test_counter = counter

        self.module_name = reports_sub_dir_name.replace('_', ' ').title()
        reports_dir_name = f"{module_order}_{reports_sub_dir_name}"
        self.reports_dir = Path.cwd() / "output" / reports_dir_name

    def run_command_test(self, test_name: str, command_args: list, stdin_input: str = None):
        """
        A generic function to run a command, save detailed output to a log file,
        and print a single summary line to the console.
        """
        current_count = self.test_counter.increment()

        sanitized_test_name = re.sub(r'[^a-zA-Z0-9]+', '_', test_name).lower()
        log_filename = f"{current_count}_{sanitized_test_name}.log"
        log_filepath = self.reports_dir / log_filename

        command = [str(self.executable_path)] + command_args
        start_time = time.monotonic()
        status = "FAIL"

        try:
            result = subprocess.run(
                command, input=stdin_input, capture_output=True, text=True,
                check=False, encoding='utf-8', errors='ignore'
            )

            with open(log_filepath, 'w', encoding='utf-8') as log_file:
                log_file.write(f"Test: {test_name}\n")
                log_file.write(f"Command: {' '.join(command)}\n")
                log_file.write(f"Exit Code: {result.returncode}\n\n")
                log_file.write("--- STDOUT ---\n")
                log_file.write(strip_ansi_codes(result.stdout) or 'None')
                log_file.write("\n\n--- STDERR ---\n")
                log_file.write(strip_ansi_codes(result.stderr) or 'None')

            if result.returncode == 0:
                status = "OK"

        except Exception as e:
            with open(log_filepath, 'w', encoding='utf-8') as log_file:
                log_file.write(f"An exception occurred while running the test: {test_name}\n")
                log_file.write(str(e))

        finally:
            duration = time.monotonic() - start_time
            log_path_str = str(log_filepath.relative_to(Path.cwd()))

            # [MODIFIED] Add color to the status text
            if status == "OK":
                status_colored = f"{Colors.GREEN}{status}{Colors.RESET}"
            else:
                status_colored = f"{Colors.RED}{status}{Colors.RESET}"

            test_info = f" -> {test_name:<15} | Log: {log_path_str}"
            status_info = f"... {status_colored} ({duration:.2f}s)"

            print(f"{test_info:<70} {status_info}")