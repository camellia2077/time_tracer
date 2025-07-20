#!/bin/bash

# --- 自动编译和打包脚本 for MSYS2/MinGW64 ---
#
# 功能:
# 1. 切换到脚本所在目录。
# 2. 创建一个独立的构建目录 (build)。
# 3. 使用 CMake 生成 Release 版本的 Makefile。
# 4. 使用 make 并行编译项目。
# 5. 使用 cpack 创建安装包 (可选)。

# --- 0. 切换到脚本所在目录 ---
cd "$(dirname "$0")"
echo "--- Switched to script directory: $(pwd)"

# 如果任何命令执行失败，立即退出脚本
set -e

# --- 变量定义 ---
BUILD_DIR="build"
BUILD_INSTALLER_FLAG="" # 用于控制是否生成安装包的 CMake flag

# --- 解析命令行参数 ---
# 例如：./build.sh --package 会生成安装包
for arg in "$@"; do
    case $arg in
        --package | -p)
            BUILD_INSTALLER_FLAG="-DBUILD_INSTALLER=ON"
            shift # 移除已处理的参数
            ;;
        *)
            # 如果有其他未识别的参数，可以处理或警告
            echo "Warning: Unknown argument '$arg' ignored."
            shift
            ;;
    esac
done

# --- 1. 清理旧的构建文件 ---
echo "--- Cleaning up previous build artifacts..."
rm -rf ./${BUILD_DIR}
echo "--- Cleanup complete."

# --- 2. 运行 CMake 来配置项目并生成 Makefile ---
echo "--- Configuring project with CMake..."
mkdir -p ./${BUILD_DIR}
cd ./${BUILD_DIR}

# 将 BUILD_INSTALLER_FLAG 传递给 CMake
cmake -S .. -B . -G "MSYS Makefiles" -D CMAKE_BUILD_TYPE=Release ${BUILD_INSTALLER_FLAG}
echo "--- CMake configuration complete."

# --- 3. 执行编译 ---
echo "--- Building the project with make..."
make -j$(nproc)
echo "--- Build complete."

# --- 4. 打包项目 (Packaging) - 新增部分 ---
# 只有当 BUILD_INSTALLER_FLAG 不为空时（即用户传入了 --package 或 -p），才运行 cpack
if [ -n "${BUILD_INSTALLER_FLAG}" ]; then
    echo "--- Creating the installation package with CPack..."
    cpack
    echo "--- Packaging complete."
else
    echo "--- Skipping CPack packaging. To create an installer, run script with --package or -p."
fi


# --- 5. 完成 ---
echo ""
echo -e "\e[32m--- Process finished successfully! ---\e[0m"
echo "--- Check the '${BUILD_DIR}' directory for:"
echo "    - Executables: time_tracker_app.exe and time_tracker_cli.exe"
if [ -n "${BUILD_INSTALLER_FLAG}" ]; then
    echo "    - Installation Package: e.g., TimeTrackerApp-0.2.0-win64.exe (exact name depends on version/system)"
fi