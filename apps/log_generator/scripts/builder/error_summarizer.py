# builder/error_summarizer.py

import re
from typing import List


def extract_errors(log_lines: List[str]) -> List[str]:
    """
    Extract core error messages from a list of log lines.
    Supports GCC/Clang style error messages.
    """
    error_pattern = re.compile(r".*:\d+:\d+: (error|fatal error):.*")
    errors = []
    for line in log_lines:
        if error_pattern.search(line):
            errors.append(line.strip())
    return errors


def print_error_summary(errors: List[str]):
    """
    Print a concise summary of detected errors.
    """
    if not errors:
        return

    print("\n" + "!" * 20 + " CORE ERRORS DETECTED " + "!" * 20)
    for err in errors:
        print(f"  [ERROR] {err}")
    print("!" * 60)
