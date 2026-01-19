# src/tt_testing/main.py
import sys
import os
from pathlib import Path

# 使用包内相对导入
from .conf.loader import load_config
from .core.engine import TestEngine
from .conf.definitions import Colors

def print_header(paths):
    """
    打印当前测试运行的关键路径配置，方便调试确认。
    """
    print(f"{Colors.CYAN}" + "="*80 + f"{Colors.RESET}")
    print(f" Running Python test script: {Path(__file__).name}")
    print(f"{Colors.CYAN}" + "="*80 + f"{Colors.RESET}")
    
    # 打印关键路径信息 (从 paths 对象中读取)
    print(f" {Colors.GREEN}[Configuration Summary]{Colors.RESET}")
    print(f"  • Source Binaries : {paths.SOURCE_EXECUTABLES_DIR}")
    print(f"  • Source Data     : {paths.SOURCE_DATA_PATH}")
    print(f"  • Test Environment: {paths.TARGET_EXECUTABLES_DIR}")
    print(f"  • Database File   : {paths.DB_DIR}")
    print(f"  • Export Output   : {paths.EXPORT_OUTPUT_DIR}")
    print(f"  • Python Logs     : {paths.PY_OUTPUT_DIR}")
    print(f"{Colors.CYAN}" + "-"*80 + f"{Colors.RESET}\n")

def main(config_path: Path = None):
    # 1. 加载配置
    # 接收从 run.py 传入的绝对路径，解决 "找不到 config.toml" 的问题
    try:
        config = load_config(config_path)
    except Exception as e:
        print(f"{Colors.RED}Config Error: {e}{Colors.RESET}")
        return 1  # 返回错误码

    # 2. 打印头信息
    print_header(config.paths)

    # 3. 初始化并运行测试引擎
    engine = TestEngine(config)
    success = engine.run()

    return 0 if success else 1