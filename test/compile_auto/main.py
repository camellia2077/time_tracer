# main.py
import os
import sys
import argparse
import time
import shutil

# --- 从 config.py 导入所有配置 ---
try:
    from config import (
        SOURCE_DIRECTORY, OUTPUT_DIRECTORY, COMPILE_TYPES,
        MARKDOWN_COMPILERS, BENCHMARK_LOOPS
    )
except ImportError:
    print("错误：无法找到或导入 config.py 文件。")
    print("请确保所有必需的配置项都已定义。")
    sys.exit(1)

# 从 internal 包中导入命令处理函数
from internal.handlers import handle_auto

def format_time(seconds):
    """将秒数格式化为 HH:MM:SS """
    seconds = int(seconds)
    hours = seconds // 3600
    minutes = (seconds % 3600) // 60
    seconds = seconds % 60
    return f"{hours:02d}:{minutes:02d}:{seconds:02d}"

def main():
    program_start_time = time.perf_counter()
    parser = argparse.ArgumentParser(
        description="一个通用的、支持并行的文档编译器（配置文件驱动）。",
        epilog="所有路径和编译选项均在 config.py 中配置。直接运行 'python main.py' 即可。"
    )
    
    parser.add_argument('--no-clean', action='store_true', help='【可选】启动时不清理旧的输出目录。')
    parser.add_argument(
        '--jobs', '-j', type=int, default=None,
        help="【可选】并行编译的任务数量 (默认: 使用所有可用的CPU核心)"
    )
    parser.add_argument('--font', type=str, default="Noto Serif SC", help="【可选】为 Pandoc 指定 CJK 字体 (默认: Noto Serif SC)")

    args = parser.parse_args()
    
    source_dir_to_process = SOURCE_DIRECTORY
    output_dir_to_process = os.path.join(os.getcwd(), OUTPUT_DIRECTORY)

    if not args.no_clean:
        if os.path.exists(output_dir_to_process):
            print(f"🧹 默认执行清理，正在删除旧的输出目录: '{output_dir_to_process}'")
            try:
                shutil.rmtree(output_dir_to_process)
                print("✅ 旧目录已成功删除。")
            except OSError as e:
                print(f"致命错误：无法删除输出目录 '{output_dir_to_process}': {e}")
                sys.exit(1)
    
    try:
        os.makedirs(output_dir_to_process, exist_ok=True)
    except OSError as e:
        print(f"致命错误：无法创建顶级输出目录 '{output_dir_to_process}': {e}")
        sys.exit(1)

    if not os.path.isdir(source_dir_to_process):
        print(f"错误：在 config.py 中配置的源路径 '{source_dir_to_process}' 不是一个有效的目录。")
        sys.exit(1)
        
    # --- 将所有配置打包，传递给核心处理器 ---
    auto_mode_args = argparse.Namespace(
        source_dir=source_dir_to_process,
        output_dir=output_dir_to_process,
        font=args.font,
        jobs=args.jobs,
        compile_types=COMPILE_TYPES,
        markdown_compilers=MARKDOWN_COMPILERS,
        benchmark_loops=BENCHMARK_LOOPS
    )
    handle_auto(auto_mode_args)
    
    program_end_time = time.perf_counter()
    print(f"\n\n🚀 程序总运行时间: {format_time(program_end_time - program_start_time)}")

if __name__ == '__main__':
    main()