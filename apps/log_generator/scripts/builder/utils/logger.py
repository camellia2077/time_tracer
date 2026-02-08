# builder/utils/logger.py
import re
import sys
from pathlib import Path


class BuildLogger:
    """Write to console and log file, stripping ANSI codes in the log."""

    def __init__(self, log_path: Path):
        self.log_path = log_path
        self.log_path.parent.mkdir(parents=True, exist_ok=True)
        if self.log_path.exists():
            self.log_path.unlink()

        self.ansi_escape = re.compile(r"\x1b\[[0-9;]*m")

    def log(self, message: str, end="\n", flush=True):
        sys.stdout.write(f"{message}{end}")
        if flush:
            sys.stdout.flush()

        clean_message = self.ansi_escape.sub("", message)
        with open(self.log_path, "a", encoding="utf-8") as handle:
            handle.write(f"{clean_message}{end}")

    def log_error(self, message: str, end="\n", flush=True):
        sys.stderr.write(f"{message}{end}")
        if flush:
            sys.stderr.flush()

        clean_message = self.ansi_escape.sub("", message)
        with open(self.log_path, "a", encoding="utf-8") as handle:
            handle.write(f"ERROR: {clean_message}{end}")
