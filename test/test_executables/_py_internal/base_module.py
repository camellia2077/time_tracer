# base_module.py
import re
import subprocess
from pathlib import Path
from . import config
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
    def __init__(self, counter: TestCounter, module_order: int, reports_sub_dir_name: str):
        self.executable_path = Path.cwd() / config.EXECUTABLE_TO_RUN
        self.source_data_path = config.SOURCE_DATA_PATH
        self.processed_data_path = Path.cwd() / config.CONVERTED_TEXT_DIR_NAME
        self.test_counter = counter
        
        reports_dir_name = f"{module_order}_{reports_sub_dir_name}"
        self.reports_dir = Path.cwd() / "output" / reports_dir_name

    def _log_to_console(self, message):
        """Helper to print messages to the console."""
        print(message)

    def run_command_test(self, test_name: str, command_args: list, stdin_input: str = None):
        """A generic function to run a command and log its output."""
        current_count = self.test_counter.increment()
        title = f" {current_count}. Test: {test_name} "
        outer_separator = "=" * 80
        inner_separator = "-" * 80

        if command_args[0] in ['-q', '-e'] and len(command_args) > 1:
            flags_part = f"_{command_args[0]}_{command_args[1]}"
        else:
            flags = [arg for arg in command_args if arg.startswith('-')]
            flags_part = "".join([f"_{flag}" for flag in flags])
        
        log_filename = f"{current_count}{flags_part}.txt"
        log_filepath = self.reports_dir / log_filename
        
        with open(log_filepath, 'w', encoding='utf-8') as log_file:
            def log_all(message):
                self._log_to_console(message)
                log_file.write(strip_ansi_codes(message) + '\n')

            log_all(outer_separator)
            log_all(f"=={title}".ljust(len(outer_separator) - 2) + "==")
            log_all(inner_separator)
            
            command = [str(self.executable_path)] + command_args
            log_all(f"Step 1: Executing Command\n  {' '.join(command)}")
            log_all(inner_separator)
            log_all(f"Step 2: Program Output and Analysis")
            
            try:
                result = subprocess.run(
                    command, input=stdin_input, capture_output=True, text=True,
                    check=False, encoding='utf-8', errors='ignore'
                )
                log_all(f"  [Exit Code]: {result.returncode}")
                log_all(f"  [STDOUT]:\n{result.stdout.strip() or 'None'}")
                log_all(f"  [STDERR]:\n{result.stderr.strip() or 'None'}")
                
                # Specific check for the pre-processing module
                if "-c" in command_args or "--convert" in command_args:
                    log_all(f"\n  [File Check]:")
                    if self.processed_data_path.exists() and self.processed_data_path.is_dir():
                        log_all(f"    ✔️ Success! Output directory '{self.processed_data_path.name}' was created.")
                    else:
                        log_all(f"    ❌ Failure! Output directory '{self.processed_data_path.name}' was NOT created.")
            
            except Exception as e:
                log_all(f"  An unknown error occurred: {e}")
            
            log_all(outer_separator + "\n")