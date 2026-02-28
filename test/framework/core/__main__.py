# src/tt_testing/__main__.py
import sys

from .main import main

if __name__ == "__main__":
    # 这里的 sys.exit 确保 main 函数返回的状态码能传给操作系统
    sys.exit(main())
