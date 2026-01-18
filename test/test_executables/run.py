# run.py
import sys
import os
from pathlib import Path

# 1. 获取项目根目录
project_root = Path(__file__).parent.resolve()

# 2. [修改] 直接将当前根目录加入 sys.path
sys.path.insert(0, str(project_root))

# 3. 导入主函数
try:
    from tt_testing.main import main
except ImportError as e:
    print(f"启动失败: {e}")
    print(f"当前的 sys.path: {sys.path}")
    sys.exit(1)

if __name__ == "__main__":
    try:
        sys.exit(main())
    except KeyboardInterrupt:
        print("\n[Interrupted by User]")
        sys.exit(130)