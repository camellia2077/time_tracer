#!/bin/bash

# --- 自动编译脚本 for MSYS2/MinGW64 ---
#
# 功能:
# 1. 切换到脚本所在目录。
# 2. 创建一个独立的构建目录 (build)。
# 3. 使用 CMake 生成 Release 版本的 Makefile。
# 4. 使用 make 并行编译项目。
# 5. 可执行文件将生成在 "build" 目录中。

# --- 0. 切换到脚本所在目录 ---
# 这确保了无论从哪里运行此脚本，所有相对路径都是正确的。
cd "$(dirname "$0")"
echo "--- Switched to script directory: $(pwd)"

# 如果任何命令执行失败，立即退出脚本
set -e

# --- 变量定义 ---
# 构建目录的名称
BUILD_DIR="build"
# 目标可执行文件名 (与 CMakeLists.txt 中的项目名称对应)
EXECUTABLE_NAME="time_tracker_app.exe"

# --- 1. 清理旧的构建文件 ---
echo "--- Cleaning up previous build artifacts..."
# -f 选项可以避免在目录不存在时报错
rm -rf ./${BUILD_DIR}
echo "--- Cleanup complete."

# --- 2. 运行 CMake 来配置项目并生成 Makefile ---
echo "--- Configuring project with CMake..."
# 创建构建目录并进入
mkdir -p ./${BUILD_DIR}
cd ./${BUILD_DIR}

# 运行 CMake
# -S .. : 指定源代码在上一级目录
# -B .  : 指定在此当前目录生成构建文件
# -G "MSYS Makefiles" : 为 MSYS 环境指定生成器
# -D CMAKE_BUILD_TYPE=Release : 设置构建类型为 Release, 以启用优化
cmake -S .. -B . -G "MSYS Makefiles" -D CMAKE_BUILD_TYPE=Release

echo "--- CMake configuration complete."

# --- 3. 执行编译 ---
echo "--- Building the project with make..."
# 使用 make 命令进行编译。-j$(nproc) 会使用所有可用的 CPU核心来加速编译。
make -j$(nproc)
echo "--- Build complete."

# --- 4. 完成 ---
echo ""
echo "--- Process finished successfully! ---"
echo "--- The executable '${EXECUTABLE_NAME}' is located in the '${BUILD_DIR}' directory."
echo "--- To run it, you may need to copy required DLLs (like sqlite3.dll) into the '${BUILD_DIR}' directory, or run it from the MSYS2 MinGW 64-bit shell."