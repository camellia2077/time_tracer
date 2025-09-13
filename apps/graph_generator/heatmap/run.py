import os
import sys

# 将应用目录添加到 Python 路径中
sys.path.insert(0, os.path.abspath(os.path.dirname(__file__)))

from heatmap_app.main import run_generator

if __name__ == "__main__":
    # 获取项目根目录 (run.py 所在的目录)
    BASE_DIRECTORY = os.path.dirname(os.path.abspath(__file__))
    run_generator(BASE_DIRECTORY)