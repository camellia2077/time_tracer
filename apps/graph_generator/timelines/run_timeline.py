import os
import sys

# 获取项目根目录 (run_timeline.py 所在的目录)
BASE_DIRECTORY = os.path.dirname(os.path.abspath(__file__))

# ✨ 核心修改：将 src 目录添加到 Python 路径中
SRC_PATH = os.path.join(BASE_DIRECTORY, "src")
sys.path.insert(0, SRC_PATH)

# 现在，因为 src 在路径中，Python 可以直接找到里面的 main 模块
from main import run_timeline_generator # type: ignore

if __name__ == "__main__":
    # 我们将项目根目录传递给函数，以便它能找到 configs 文件夹
    run_timeline_generator(BASE_DIRECTORY)