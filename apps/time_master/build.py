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

# ==================== [核心修改] ====================
#
#          编译器警告级别开关 (Warning Level Switch)
#
#   - 设置为 1: 启用基础警告 (-Wall)。
#
#   - 设置为 2: 启用扩展警告 (-Wall -Wextra -Wpedantic)。
#               推荐用于常规开发。
#
#   - 设置为 3: 启用严格的开发者模式 (级别2 + 将所有警告视为错误)。
#               推荐在提交代码前使用。
#
WARNING_LEVEL = 1
#
# ====================================================


# --- 全局构建配置 ---
COMPILER = "default"

def print_header(message):
    """打印带有标题格式的日志消息"""
    print(f"{Color.HEADER}{Color.BOLD}--- {message} ---{Color.ENDC}")

def parse_arguments(args):
    """解析命令行参数"""
    print_header("Parsing command-line arguments...")
    global COMPILER
    
    should_install = 'install' in args
    if should_install:
        args.remove('install')
        should_clean = True
        should_package = True
    else:
        should_clean = 'clean' in args
        should_package = '--package' in args or '-p' in args

    if '--gcc' in args:
        COMPILER = 'gcc'
        args.remove('--gcc')
        print("--- Compiler override: GCC will be used.")
    elif '--clang' in args:
        COMPILER = 'clang'
        args.remove('--clang')
        print("--- Compiler override: Clang will be used.")

    cmake_args = [arg for arg in args if arg.startswith('-D')]
    if cmake_args:
        print(f"--- Found manual CMake arguments: {' '.join(cmake_args)}")
    
    known_args = {'clean', '--package', '-p', 'install', '--gcc', '--clang'}
    remaining_args = [arg for arg in args if arg not in known_args and not arg.startswith('-D')]
    
    if remaining_args:
        for arg in remaining_args:
            print(f"{Color.WARNING}Warning: Unknown argument '{arg}' ignored.{Color.ENDC}")
            
    return {"clean": should_clean, "package": should_package, "install": should_install}, cmake_args

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

def run_cmake(should_package, cmake_args):
    """配置项目 (CMake)"""
    print_header("Configuring project with CMake for Ninja...")
    
    cmake_env = os.environ.copy()
    if COMPILER == 'gcc':
        print("--- Setting environment for GCC compiler (CC=gcc, CXX=g++).")
        cmake_env['CC'] = 'gcc'
        cmake_env['CXX'] = 'g++'
    elif COMPILER == 'clang':
        print("--- Setting environment for Clang compiler (CC=clang, CXX=clang++).")
        cmake_env['CC'] = 'clang'
        cmake_env['CXX'] = 'clang++'

    cmake_command = [
        "cmake", "-S", "..", "-B", ".", "-G", "Ninja",
        "-D", "CMAKE_BUILD_TYPE=Release"
    ]
    if should_package:
        cmake_command.append("-DBUILD_INSTALLER=ON")
    
    # --- [核心修改] ---
    # 根据 WARNING_LEVEL 开关自动添加对应的CMake标志
    if WARNING_LEVEL in [1, 2, 3]:
        print(f"{Color.OKBLUE}--- Setting warning level to {WARNING_LEVEL}. ---{Color.ENDC}")
        cmake_command.append(f"-DWARNING_LEVEL={WARNING_LEVEL}")
    else:
        print(f"{Color.WARNING}--- Invalid WARNING_LEVEL '{WARNING_LEVEL}'. Defaulting to level 2. ---{Color.ENDC}")
        cmake_command.append("-DWARNING_LEVEL=2")
        
    cmake_command.extend(cmake_args)
    
    result = subprocess.run(cmake_command, check=True, capture_output=True, text=True, env=cmake_env)
    print("--- CMake configuration complete.")

# ... (文件的其余部分保持不变)
def run_build():
    """执行编译 (Ninja)"""
    print_header("Building the project with Ninja...")
    subprocess.run(["ninja"], check=True)
    print("--- Build complete.")

def run_cpack():
    """打包项目 (CPack)"""
    print_header("Creating the installation package with CPack...")
    subprocess.run(["cpack"], check=True)
    print("--- Packaging complete.")
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
    """主函数"""
    start_time = time.monotonic()
    project_dir = Path(__file__).resolve().parent
    build_dir_name = "build"
    installer_file = None
    
    os.chdir(project_dir)
    print_header(f"Switched to project directory: {os.getcwd()}")
    
    try:
        options, cmake_args = parse_arguments(sys.argv[1:])
        build_dir = prepare_build_directory(project_dir, build_dir_name, options["clean"])
        os.chdir(build_dir)
        
        run_cmake(options["package"], cmake_args)
        run_build()

        if options["package"]:
            installer_file = run_cpack()
        if options["install"]:
            run_installer(installer_file)

    except subprocess.CalledProcessError as e:
        print(f"\n{Color.FAIL}!!! A build step failed with exit code {e.returncode}.{Color.ENDC}")
        if e.stdout: print(f"{Color.FAIL}Output:\n{e.stdout}{Color.ENDC}")
        if e.stderr: print(f"{Color.FAIL}Error output:\n{e.stderr}{Color.ENDC}")
        sys.exit(e.returncode)
    except Exception as e:
        print(f"\n{Color.FAIL}!!! An unexpected error occurred: {e}{Color.ENDC}")
        sys.exit(1)
    finally:
        end_time = time.monotonic()
        duration = int(end_time - start_time)
        minutes, seconds = divmod(duration, 60)

        print("\n" + "="*60)
        print(f"{Color.OKGREEN}{Color.BOLD}Process finished successfully!{Color.ENDC}")
        print(f"Artifacts are in the '{build_dir_name}' directory.")
        
        # 重新解析以获取最终状态
        options, _ = parse_arguments(sys.argv[1:]) 
        if options["package"] and not options["install"]:
            print("Installation package has also been created.")
        if options["install"]:
            print("Project has been built, packaged, and the installer was launched.")
        
        print("-" * 60)
        print(f"Total time elapsed: {Color.BOLD}{minutes}m {seconds}s{Color.ENDC}")
        print("="*60)

if __name__ == "__main__":
    main()