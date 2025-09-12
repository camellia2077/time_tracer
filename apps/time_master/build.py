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

def parse_arguments(args):
    """解析命令行参数"""
    print_header("Parsing command-line arguments...")
    
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
            
    return {"clean": should_clean, "package": should_package, "install": should_install}

def prepare_build_directory(project_dir, build_dir_name, should_clean):
    """创建或清理构建目录"""
    build_dir = project_dir / build_dir_name
    
    if should_clean:
        print_header("'clean' or 'install' provided. Cleaning previous build artifacts...")
        if build_dir.exists():
            shutil.rmtree(build_dir)
        print("--- Cleanup complete.")

    build_dir.mkdir(exist_ok=True)
    return build_dir

def run_cmake(should_package):
    """配置项目 (CMake)"""
    print_header("Configuring project with CMake for Ninja...")
    cmake_command = [
        "cmake", "-S", "..", "-B", ".", "-G", "Ninja",  # [修改] 使用 Ninja 生成器
        "-D", "CMAKE_BUILD_TYPE=Release"
    ]
    if should_package:
        cmake_command.append("-DBUILD_INSTALLER=ON")
    
    # 隐藏输出来保持简洁，错误时仍会显示
    result = subprocess.run(cmake_command, check=True, capture_output=True, text=True)
    print("--- CMake configuration complete.")

def run_build():
    """执行编译 (Ninja)"""
    print_header("Building the project with Ninja...")
    # [修改] 使用 'ninja' 命令，它会自动并行编译
    subprocess.run(["ninja"], check=True)
    print("--- Build complete.")

def run_cpack():
    """打包项目 (CPack)"""
    print_header("Creating the installation package with CPack...")
    subprocess.run(["cpack"], check=True)
    print("--- Packaging complete.")
    
    # 查找生成的安装程序文件
    installers = list(Path.cwd().glob("TimeTrackerApp-*-win64.exe"))
    if not installers:
        raise FileNotFoundError("CPack finished, but no installer executable was found.")
    
    installer_file = installers[0]
    print(f"--- Installer found: {installer_file.name}")
    return installer_file

def run_installer(installer_file):
    """运行安装程序"""
    if not installer_file:
         raise Exception("Cannot run installer, as it was not created.")
    print_header("Launching the installer...")
    subprocess.run([installer_file], check=True)
    print("--- Installer process has been launched.")

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
    
    # 在主逻辑之外预先切换到项目目录
    os.chdir(project_dir)
    print_header(f"Switched to project directory: {os.getcwd()}")
    
    try:
        # 1. 解析参数
        options = parse_arguments(sys.argv[1:])

        # 2. 准备构建目录
        build_dir = prepare_build_directory(project_dir, build_dir_name, options["clean"])
        os.chdir(build_dir)
        
        # 3. 运行 CMake
        run_cmake(options["package"])

        # 4. 运行编译
        run_build()

        # 5. 打包
        if options["package"]:
            installer_file = run_cpack()

        # 6. 安装
        if options["install"]:
            run_installer(installer_file)

    except subprocess.CalledProcessError as e:
        print(f"\n{Color.FAIL}!!! A build step failed with exit code {e.returncode}.{Color.ENDC}")
        # 错误输出现在在 e.stdout 和 e.stderr 中
        if e.stdout:
            print(f"{Color.FAIL}Output:\n{e.stdout}{Color.ENDC}")
        if e.stderr:
            print(f"{Color.FAIL}Error output:\n{e.stderr}{Color.ENDC}")
        sys.exit(e.returncode)
    except Exception as e:
        print(f"\n{Color.FAIL}!!! An unexpected error occurred: {e}{Color.ENDC}")
        sys.exit(1)
    finally:
        # 7. 结束并报告时间
        end_time = time.monotonic()
        duration = int(end_time - start_time)
        minutes, seconds = divmod(duration, 60)

        print("\n" + "="*60)
        print(f"{Color.OKGREEN}{Color.BOLD}Process finished successfully!{Color.ENDC}")
        print(f"Artifacts are in the '{build_dir_name}' directory.")
        
        # 使用解析后的选项来决定最终打印的消息
        options = parse_arguments(sys.argv[1:]) # 重新解析以获取最终状态
        if options["package"] and not options["install"]:
            print("Installation package has also been created.")
        if options["install"]:
            print("Project has been built, packaged, and the installer was launched.")
        
        print("-" * 60)
        print(f"Total time elapsed: {Color.BOLD}{minutes}m {seconds}s{Color.ENDC}")
        print("="*60)

if __name__ == "__main__":
    main()