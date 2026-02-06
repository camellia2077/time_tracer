# builder/utils/logger.py
import sys
import os
import re
from pathlib import Path

class BuildLogger:
    """处理同时向控制台和日志文件输出内容，并剥离日志文件中的 ANSI 颜色代码。"""
    
    def __init__(self, log_path: Path):
        self.log_path = log_path
        # 确保目录存在
        self.log_path.parent.mkdir(parents=True, exist_ok=True)
        # 每次运行先删除旧日志
        if self.log_path.exists():
            self.log_path.unlink()
        
        # 预编译 ANSI 恢复正则表达式
        self.ansi_escape = re.compile(r'\x1b\[[0-9;]*m')

    def log(self, message: str, end="\n", flush=True):
        """同步写入控制台和文件"""
        # 1. 写入控制台 (保留颜色)
        sys.stdout.write(f"{message}{end}")
        if flush:
            sys.stdout.flush()

        # 2. 写入文件 (剥离颜色)
        clean_message = self.ansi_escape.sub('', message)
        with open(self.log_path, "a", encoding="utf-8") as f:
            f.write(f"{clean_message}{end}")

    def log_error(self, message: str, end="\n", flush=True):
        """写入错误流"""
        sys.stderr.write(f"{message}{end}")
        if flush:
            sys.stderr.flush()

        clean_message = self.ansi_escape.sub('', message)
        with open(self.log_path, "a", encoding="utf-8") as f:
            f.write(f"ERROR: {clean_message}{end}")
