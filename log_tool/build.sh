#!/bin/bash

# 切换到脚本所在的目录
# 这确保了无论从哪个目录执行脚本，CMake 都能找到 CMakeLists.txt
SCRIPT_DIR=$(dirname "$0")
cd "$SCRIPT_DIR" || { echo "Error: Could not change to script directory." ; exit 1; }

echo "Current directory: $(pwd)"

BUILD_DIR="build"

# 检查是否存在 build 目录
if [ -d "$BUILD_DIR" ]; then
    echo "Found existing '$BUILD_DIR' directory. Deleting it..."
    rm -rf "$BUILD_DIR"
    if [ $? -ne 0 ]; then
        echo "Error: Failed to remove '$BUILD_DIR' directory."
        exit 1
    fi
else
    echo "No existing '$BUILD_DIR' directory found."
fi

# 创建新的 build 目录
echo "Creating new '$BUILD_DIR' directory..."
mkdir "$BUILD_DIR"
if [ $? -ne 0 ]; then
    echo "Error: Failed to create '$BUILD_DIR' directory."
    exit 1
fi

# 进入 build 目录
cd "$BUILD_DIR" || { echo "Error: Could not change to build directory." ; exit 1; }

# 运行 CMake 配置
# 使用 MinGW Makefiles 生成器，适用于 MSYS2/MinGW-w64 环境
echo "Running CMake configuration..."
cmake .. -G "MinGW Makefiles"

# 检查 CMake 配置是否成功
if [ $? -ne 0 ]; then
    echo "Error: CMake configuration failed."
    exit 1
fi

# 运行 make 编译
# 在 MinGW 环境下通常使用 mingw32-make 或 make
echo "Running make to compile the project..."
mingw32-make

# 检查编译是否成功
if [ $? -ne 0 ]; then
    echo "Error: Project compilation failed."
    exit 1
fi

echo "Build completed successfully!"
echo "Executable should be in: $(pwd)/log_tool.exe (or log_tool)"
