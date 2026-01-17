# main.py
import sys
import os
from pathlib import Path
from conf.loader import load_config
from core.engine import TestEngine

def print_header(paths):
    """打印基础信息"""
    print("\n" + "="*50)
    print(f" Running Python test script: {Path(__file__).name}")
    print(f" Input data path: {paths.SOURCE_DATA_PATH}")
    print(f" Expecting processed folder: {paths.PROCESSED_DATA_DIR_NAME}")
    print("="*50 + "\n")

def main():
    os.system('') 
    
    # 1. 加载配置
    try:
        # 直接获取 config 对象
        config = load_config()
    except Exception as e:
        print(f"\033[91mConfig Error: {e}\033[0m")
        sys.exit(1)

    # 访问属性更加直观
    print_header(config.paths)

    # 2. 实例化引擎 (只需传一个参数)
    engine = TestEngine(config)
    success = engine.run()

    # 3. 根据结果退出
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()