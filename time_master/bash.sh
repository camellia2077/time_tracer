#!/bin/bash

# --- 自动编译和打包脚本 for MSYS2/MinGW64 ---
#
# 功能:
# 1. 切换到脚本所在目录。
# 2. 创建一个独立的构建目录 (build)。
# 3. 使用 CMake 生成 Release 版本的 Makefile。
# 4. 使用 make 并行编译项目。
# 5. 使用 cpack 创建安装包。

# --- 0. 切换到脚本所在目录 ---
cd "$(dirname "$0")"
echo "--- Switched to script directory: $(pwd)"

# 如果任何命令执行失败，立即退出脚本
set -e

# --- 变量定义 ---
BUILD_DIR="build"

# --- 1. 清理旧的构建文件 ---
echo "--- Cleaning up previous build artifacts..."
rm -rf ./${BUILD_DIR}
echo "--- Cleanup complete."

# --- 2. 运行 CMake 来配置项目并生成 Makefile ---
echo "--- Configuring project with CMake..."
mkdir -p ./${BUILD_DIR}
cd ./${BUILD_DIR}

cmake -S .. -B . -G "MSYS Makefiles" -D CMAKE_BUILD_TYPE=Release
echo "--- CMake configuration complete."

# --- 3. 执行编译 ---
echo "--- Building the project with make..."
make -j$(nproc)
echo "--- Build complete."

# --- 4. 打包项目 (Packaging) - 新增部分 ---
echo "--- Creating the installation package with CPack..."
cpack
echo "--- Packaging complete."


# --- 5. 完成 ---
echo ""
echo -e "\e[32m--- Process finished successfully! ---\e[0m"
echo "--- Check the '${BUILD_DIR}' directory for:"
echo "    - Executables: time_tracker_app.exe and time_tracker_command.exe"
echo "    - Installation Package: e.g., TimeTrackerApp-1.0.0-win64.exe"