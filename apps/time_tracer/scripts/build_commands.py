import os
import re
import subprocess
import sys
from pathlib import Path
from build_config import config
from build_ui import print_header

def run_cmake(should_package, cmake_args, compiler):
    """配置项目 (CMake)"""
    print_header("Configuring project with CMake for Ninja...")
    
    cmake_env = os.environ.copy()
    if compiler == 'gcc':
        print("--- Setting environment for GCC compiler (CC=gcc, CXX=g++).")
        cmake_env['CC'] = 'gcc'
        cmake_env['CXX'] = 'g++'
    elif compiler == 'clang':
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
        
    # ==================== [核心修改] ====================
    # 根据配置设置LTO选项
    lto_option_str = "ON" if config.ENABLE_LTO else "OFF"
    lto_status_msg = "enabled" if config.ENABLE_LTO else "disabled"
    print(f"{config.OKBLUE}--- Link-Time Optimization (LTO) is {lto_status_msg}. ---{config.ENDC}")
    cmake_command.append(f"-DENABLE_LTO={lto_option_str}")
    # ====================================================
        
    cmake_command.extend(cmake_args)
    
    process = subprocess.Popen(cmake_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, env=cmake_env)
    
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
    """执行编译 (Ninja)"""
    print_header("Building the project with Ninja...")
    
    process = subprocess.Popen(["ninja"], stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, encoding='utf-8')

    for line in process.stdout:
        if re.search(r'error:', line, re.IGNORECASE):
            sys.stdout.write(f"{config.FAIL}{line}{config.ENDC}")
        elif re.search(r'warning:', line, re.IGNORECASE):
            sys.stdout.write(f"{config.WARNING}{line}{config.ENDC}")
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