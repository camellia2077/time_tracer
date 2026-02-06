# run.py
import sys
import os
from pathlib import Path

# 1. 获取项目根目录 (run.py 所在的文件夹)
project_root = Path(__file__).parent.resolve()

# 2. 直接将当前根目录加入 sys.path
sys.path.insert(0, str(project_root))

# 3. 导入主函数
try:
    from executables_testing.main import main
except ImportError as e:
    print(f"启动失败: {e}")
    sys.exit(1)

if __name__ == "__main__":
    try:
        # 解析命令行参数
        build_dir = None
        if "--build-dir" in sys.argv:
            idx = sys.argv.index("--build-dir")
            if idx + 1 < len(sys.argv):
                build_dir = sys.argv[idx + 1]
        
        # [关键修改] 计算 config.toml 的绝对路径，并传入 main
        config_file_path = project_root / "config.toml"
        
        sys.exit(main(config_path=config_file_path, build_dir_name=build_dir))
        
    except KeyboardInterrupt:
        print("\n[Interrupted by User]")
        sys.exit(130)