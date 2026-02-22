# config.py
import sys
from internal.config_loader import load_config

# --- 定义用于存放配置的类 ---
class Paths:
    SOURCE_DIRECTORY = ""
    OUTPUT_DIRECTORY = ""

class Compilation:
    TYPES = []
    INCREMENTAL = True
    CLEAN_OUTPUT_DEFAULT = False

class Benchmark:
    COMPILERS = []
    LOOPS = 3

# --- 从 config.toml 加载配置 ---
try:
    load_config(Paths, Compilation, Benchmark)
except (FileNotFoundError, RuntimeError) as e:
    print(f"错误: {e}")
    sys.exit(1)