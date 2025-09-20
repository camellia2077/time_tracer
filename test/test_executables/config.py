# config.py
from pathlib import Path
from _py_internal.config_loader import load_config

# --- ANSI Color Codes ---
class Colors:
    """A class to hold ANSI color codes for colored console output."""
    CYAN = '\033[96m'
    GREEN = '\033[92m'
    RED = '\033[91m'
    RESET = '\033[0m'

# --- Configuration Classes (to be populated from TOML) ---
class Paths:
    pass

class CLINames:
    pass

class TestParams:
    pass

class Cleanup:
    pass

class RunControl:
    """此类用于存放运行控制相关的配置。"""
    pass

# 调用加载函数，填充类
load_config(Paths, CLINames, TestParams, Cleanup, RunControl)