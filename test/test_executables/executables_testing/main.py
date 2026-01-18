# src/tt_testing/main.py
import sys
import os
from pathlib import Path

# 使用包内相对导入 (推荐)
from .conf.loader import load_config
from .core.engine import TestEngine

def print_header(paths):
    # ... (保持原样) ...
    print(f" Running Python test script: {Path(__file__).name}")
    # ...

def main():
    # 1. 加载配置
    # 注意：现在代码在 src/tt_testing/ 下，config.toml 在项目根目录 (../../config.toml)
    # 或者，我们假设用户是在项目根目录运行命令，那么 Path.cwd() 就是根目录
    try:
        # load_config 内部默认找 "config.toml"，只要在根目录运行即可
        config = load_config()
    except Exception as e:
        print(f"\033[91mConfig Error: {e}\033[0m")
        return 1  # 返回错误码而不是直接 sys.exit，交给 __main__ 处理

    print_header(config.paths)

    engine = TestEngine(config)
    success = engine.run()

    return 0 if success else 1