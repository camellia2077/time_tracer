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
    - 'clean': 清理构建目录。
    - '--package' or '-p': 创建安装包。
    - 'install': 创建并运行安装包。
    """
    start_time = time.monotonic()
    project_dir = Path(__file__).resolve().parent
    build_dir_name = "build"
    installer_file = None

    try:
        # --- 0. 准备工作 ---
        os.chdir(project_dir)
        print_header(f"Switched to project directory: {os.getcwd()}")

        # --- 1. 解析命令行参数 ---
        args = sys.argv[1:]
        
        # 'install' 命令隐含了 'clean' 和 '--package'
        should_install = 'install' in args
        if should_install:
            args.remove('install')
            should_clean = True
            should_package = True
        else:
            should_clean = 'clean' in args
            should_package = '--package' in args or '-p' in args
        
        if 'clean' in args: args.remove('clean')
        if '--package' in args: args.remove('--package')
        if '-p' in args: args.remove('-p')
        
        if args:
            for arg in args:
                print(f"{Color.WARNING}Warning: Unknown argument '{arg}' ignored.{Color.ENDC}")

        # --- 2. 创建并进入构建目录 ---
        build_dir = Path(build_dir_name)
        build_dir.mkdir(exist_ok=True)
        os.chdir(build_dir)
        
        # --- 3. 如果需要，执行清理 ---
        if should_clean:
            print_header("'clean' or 'install' provided. Cleaning previous build artifacts...")
            # A more robust clean: remove the entire directory content
            os.chdir(project_dir)
            if build_dir.exists():
                shutil.rmtree(build_dir)
            build_dir.mkdir(exist_ok=True)
            os.chdir(build_dir)
            print("--- Cleanup complete.")

        # --- 4. 配置项目 (CMake) ---
        print_header("Configuring project with CMake...")
        cmake_command = [
            "cmake", "-S", "..", "-B", ".", "-G", "MSYS Makefiles",
            "-D", "CMAKE_BUILD_TYPE=Release"
        ]
        if should_package:
            cmake_command.append("-DBUILD_INSTALLER=ON")
        
        subprocess.run(cmake_command, check=True, capture_output=True, text=True)
        print("--- CMake configuration complete.")

        # --- 5. 执行编译 (Make) ---
        print_header("Building the project with make...")
        cpu_cores = os.cpu_count() or 1
        subprocess.run(["make", f"-j{cpu_cores}"], check=True)
        print("--- Build complete.")

        # --- 6. 打包项目 (CPack) ---
        if should_package:
            print_header("Creating the installation package with CPack...")
            subprocess.run(["cpack"], check=True)
            print("--- Packaging complete.")
            
            # Find the installer file for the next step
            installers = list(Path.cwd().glob("TimeTrackerApp-*-win64.exe"))
            if not installers:
                raise FileNotFoundError("CPack finished, but no installer executable was found.")
            installer_file = installers[0]
            print(f"--- Installer found: {installer_file.name}")

        # --- 7. (新增) 运行安装程序 ---
        if should_install:
            print_header("Launching the installer...")
            if not installer_file:
                 raise Exception("Cannot run installer, as it was not created.")
            subprocess.run([installer_file], check=True)
            print("--- Installer process has been launched.")


    except subprocess.CalledProcessError as e:
        print(f"\n{Color.FAIL}!!! A build step failed with exit code {e.returncode}.{Color.ENDC}")
        if e.stderr:
            print(f"{Color.FAIL}Error output:\n{e.stderr}{Color.ENDC}")
        sys.exit(e.returncode)
    except Exception as e:
        print(f"\n{Color.FAIL}!!! An unexpected error occurred: {e}{Color.ENDC}")
        sys.exit(1)
    finally:
        # --- 8. 结束并报告时间 ---
        end_time = time.monotonic()
        duration = int(end_time - start_time)
        minutes, seconds = divmod(duration, 60)

        print("\n" + "="*60)
        print(f"{Color.OKGREEN}{Color.BOLD}Process finished successfully!{Color.ENDC}")
        print(f"Artifacts are in the '{build_dir_name}' directory.")
        if should_package and not should_install:
            print("Installation package has also been created.")
        if should_install:
            print("Project has been built, packaged, and the installer was launched.")
        print("-" * 60)
        print(f"Total time elapsed: {Color.BOLD}{minutes}m {seconds}s{Color.ENDC}")
        print("="*60)


if __name__ == "__main__":
    main()