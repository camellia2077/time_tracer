#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
from pathlib import Path

# 1. 路径引导：确保能找到 'builder' 包
# 获取 scripts/ 目录
SCRIPT_DIR = Path(__file__).resolve().parent
# 将 scripts/ 加入 python 搜索路径，以便 import builder
sys.path.append(str(SCRIPT_DIR))

try:
    from builder.core import BuildPipeline
except ImportError as e:
    print(f"Critical Error: 无法加载构建核心模块。\n{e}")
    sys.exit(1)

def main():
    # 定义项目根目录 (假设 scripts/ 在项目根目录下)
    project_dir = SCRIPT_DIR.parent
    
    # 实例化流水线
    pipeline = BuildPipeline(project_dir=project_dir, script_dir=SCRIPT_DIR)
    
    # 传入命令行参数 (sys.argv[1:] 去掉脚本名称)
    pipeline.run(sys.argv[1:])

if __name__ == "__main__":
    main()