#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import shutil
import subprocess
import sys
import time
from pathlib import Path

# --- 控制台颜色定义 ---
class Color:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'

def print_header(message):
    """打印带有标题格式的日志消息"""
    print(f"{Color.HEADER}{Color.BOLD}--- {message} ---{Color.ENDC}")

def main():
    """
    一个完整的C++项目构建脚本，使用Python实现。
    功能包括：
    - 支持 'clean' 参数来清理构建目录。
    - 支持 '--package' 或 '-p' 参数来创建安装包。
    - 使用CMake和make进行配置和编译。
    - 记录并显示总构建时间。
    """
    start_time = time.monotonic()

    try:
        # --- 0. 准备工作 ---
        # 切换到脚本所在的项目根目录
        project_dir = Path(__file__).resolve().parent
        os.chdir(project_dir)
        print_header(f"Switched to project directory: {os.getcwd()}")

        # --- 1. 解析命令行参数 ---
        args = sys.argv[1:]
        should_clean = 'clean' in args
        should_package = '--package' in args or '-p' in args
        
        if should_clean:
            args.remove('clean')
        if '--package' in args:
            args.remove('--package')
        if '-p' in args:
            args.remove('-p')
        
        if args: # 检查是否有未识别的参数
            for arg in args:
                print(f"{Color.WARNING}Warning: Unknown argument '{arg}' ignored.{Color.ENDC}")

        # --- 2. 创建并进入构建目录 ---
        build_dir = Path("build")
        build_dir.mkdir(exist_ok=True)
        os.chdir(build_dir)
        
        # --- 3. 如果需要，执行清理 ---
        if should_clean:
            print_header("'clean' argument provided. Cleaning previous build artifacts...")
            if Path("Makefile").exists():
                subprocess.run(["make", "clean"], check=True)
                print("--- Cleanup complete.")
            else:
                print("--- Makefile not found, skipping clean (first build?).")

        # --- 4. 配置项目 (CMake) ---
        print_header("Configuring project with CMake...")
        cmake_command = [
            "cmake",
            "-S", "..",
            "-B", ".",
            "-G", "MSYS Makefiles",
            "-D", "CMAKE_BUILD_TYPE=Release"
        ]
        if should_package:
            cmake_command.append("-DBUILD_INSTALLER=ON")
        
        subprocess.run(cmake_command, check=True, capture_output=True, text=True)
        print("--- CMake configuration complete.")

        # --- 5. 执行编译 (Make) ---
        print_header("Building the project with make...")
        # 获取CPU核心数以进行并行编译
        cpu_cores = os.cpu_count() or 1
        subprocess.run(["make", f"-j{cpu_cores}"], check=True)
        print("--- Build complete.")

        # --- 6. 打包项目 (CPack) ---
        if should_package:
            print_header("Creating the installation package with CPack...")
            subprocess.run(["cpack"], check=True)
            print("--- Packaging complete.")

    except subprocess.CalledProcessError as e:
        print(f"\n{Color.FAIL}!!! A build step failed with exit code {e.returncode}.{Color.ENDC}")
        # 如果有编译错误，打印出来
        if e.stderr:
            print(f"{Color.FAIL}Error output:\n{e.stderr}{Color.ENDC}")
        sys.exit(e.returncode)
    except Exception as e:
        print(f"\n{Color.FAIL}!!! An unexpected error occurred: {e}{Color.ENDC}")
        sys.exit(1)
    finally:
        # --- 7. 结束并报告时间 ---
        end_time = time.monotonic()
        duration = int(end_time - start_time)
        minutes, seconds = divmod(duration, 60)

        print("\n" + "="*60)
        print(f"{Color.OKGREEN}{Color.BOLD}Process finished successfully!{Color.ENDC}")
        print(f"Executables are in the '{build_dir.name}' directory.")
        if should_package:
            print("Installation package has also been created.")
        print("-" * 60)
        print(f"Total time elapsed: {Color.BOLD}{minutes}m {seconds}s{Color.ENDC}")
        print("="*60)


if __name__ == "__main__":
    main()