# builder/steps/commands.py

import os
import re
import subprocess
import sys
from pathlib import Path

from ..ui.console import print_header
from ..ui.colors import AnsiColors 

# [修改] 增加 no_tidy 参数，默认为 False
def run_cmake(should_package, cmake_args, compiler, config, no_opt=False, no_tidy=False):
    """
    config: AppConfig 实例，通过参数传入
    """
    print_header("Configuring project with CMake for Ninja...")
    
    cmake_env = os.environ.copy()
    if compiler == 'gcc':
        cmake_env['CC'] = 'gcc'; cmake_env['CXX'] = 'g++'
    elif compiler == 'clang':
        cmake_env['CC'] = 'clang'; cmake_env['CXX'] = 'clang++'

    cmake_command = ["cmake", "-S", "..", "-B", ".", "-G", "Ninja", "-D", "CMAKE_BUILD_TYPE=Release"]
    
    if should_package:
        cmake_command.append("-DBUILD_INSTALLER=ON")
    
    if no_opt:
        print(f"{AnsiColors.WARNING}--- Optimizations are DISABLED for faster compilation. ---{AnsiColors.ENDC}")
        cmake_command.append("-DDISABLE_OPTIMIZATION=ON")
    else:
        cmake_command.append("-DDISABLE_OPTIMIZATION=OFF")

    # 使用传入的 config 对象
    warning_level = config.WARNING_LEVEL
    cmake_command.append(f"-DWARNING_LEVEL={warning_level}")

    # LTO 设置
    lto_option = "ON" if config.ENABLE_LTO else "OFF"
    print(f"{AnsiColors.OKBLUE}--- LTO is {'enabled' if config.ENABLE_LTO else 'disabled'}. ---{AnsiColors.ENDC}")
    cmake_command.append(f"-DENABLE_LTO={lto_option}")
    
    # [新增] Clang-Tidy 控制逻辑
    if no_tidy:
        print(f"{AnsiColors.WARNING}--- Clang-Tidy static analysis is DISABLED. ---{AnsiColors.ENDC}")
        cmake_command.append("-DENABLE_CLANG_TIDY=OFF")
    else:
        # 显式开启，或者依赖 CMakeLists.txt 的默认值 (通常为了明确起见，建议显式传 ON)
        cmake_command.append("-DENABLE_CLANG_TIDY=ON")
        
    cmake_command.extend(cmake_args)
    
    # 执行 CMake
    process = subprocess.Popen(
        cmake_command, 
        stdout=subprocess.PIPE, 
        stderr=subprocess.PIPE, 
        text=True, 
        env=cmake_env,
        encoding='utf-8',
        errors='replace'
    )
    
    for line in process.stdout:
        sys.stdout.write(line)

    stderr_output = ""
    for line in process.stderr:
        stderr_output += line
        sys.stderr.write(line)

    process.wait()
    if process.returncode != 0:
        raise subprocess.CalledProcessError(process.returncode, cmake_command, output=None, stderr=stderr_output)
        
    print("--- CMake configuration complete.")

def run_build():
    """执行编译"""
    print_header("Building with Ninja...")
    process = subprocess.Popen(
        ["ninja"], 
        stdout=subprocess.PIPE, 
        stderr=subprocess.STDOUT, 
        text=True,
        encoding='utf-8',       # 编译器通常输出 UTF-8
        errors='replace'        # 遇到乱码不要崩溃，打印个问号就行
    )
    for line in process.stdout:
        # 使用 AnsiColors 直接处理高亮
        if re.search(r'error:', line, re.IGNORECASE):
            sys.stdout.write(f"{AnsiColors.FAIL}{line}{AnsiColors.ENDC}")
        elif re.search(r'warning:', line, re.IGNORECASE):
            sys.stdout.write(f"{AnsiColors.WARNING}{line}{AnsiColors.ENDC}")
        else:
            sys.stdout.write(line)
    
    process.wait()
    if process.returncode != 0:
        raise subprocess.CalledProcessError(process.returncode, "ninja")
    print("--- Build complete.")

def run_cpack():
    """打包项目 (CPack)"""
    print_header("Creating the installation package with CPack...")
    subprocess.run(["cpack"], check=True)
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