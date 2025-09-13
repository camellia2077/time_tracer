import os
import shutil
import subprocess
from pathlib import Path
from build_config import config
import re
import sys

# --- 全局构建状态 ---
# 从配置文件初始化，但可以被命令行参数覆盖
COMPILER = config.COMPILER

def print_header(message):
    """打印带有标题格式的日志消息"""
    print(f"{config.HEADER}{config.BOLD}--- {message} ---{config.ENDC}")

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
            print(f"{config.WARNING}Warning: Unknown argument '{arg}' ignored.{config.ENDC}")
            
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
    
    warning_level = config.WARNING_LEVEL
    if warning_level in [1, 2, 3]:
        print(f"{config.OKBLUE}--- Setting warning level to {warning_level}. ---{config.ENDC}")
        cmake_command.append(f"-DWARNING_LEVEL={warning_level}")
    else:
        print(f"{config.WARNING}--- Invalid WARNING_LEVEL '{warning_level}'. Defaulting to level 2. ---{config.ENDC}")
        cmake_command.append("-DWARNING_LEVEL=2")
        
    cmake_command.extend(cmake_args)
    
    # Use Popen to stream and colorize the output
    process = subprocess.Popen(cmake_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, env=cmake_env)
    
    # Print stdout directly
    for line in process.stdout:
        sys.stdout.write(line)

    # Process stderr for errors
    stderr_output = ""
    for line in process.stderr:
        stderr_output += line
        sys.stderr.write(line)

    process.wait()
    if process.returncode != 0:
        raise subprocess.CalledProcessError(process.returncode, cmake_command, output=None, stderr=stderr_output)
        
    print("--- CMake configuration complete.")

def run_build():
    """执行编译 (Ninja)"""
    print_header("Building the project with Ninja...")
    
    # Use Popen to stream the output
    process = subprocess.Popen(["ninja"], stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, encoding='utf-8')

    for line in process.stdout:
        # Check for error and warning keywords to apply color
        if re.search(r'error:', line, re.IGNORECASE):
            sys.stdout.write(f"{config.FAIL}{line}{config.ENDC}")
        elif re.search(r'warning:', line, re.IGNORECASE):
            sys.stdout.write(f"{config.WARNING}{line}{config.ENDC}")
        else:
            sys.stdout.write(line)
    
    process.wait()
    
    if process.returncode != 0:
        # The CalledProcessError will be caught by the main exception handler
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