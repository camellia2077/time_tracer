#!/bin/bash

# 如果任何命令执行失败，立即退出脚本
set -e

# 切换到项目根目录
cd "$(dirname "$0")/../../.."
echo "--- 已切换到项目根目录: $(pwd)"

echo "--- 正在执行 Python 构建脚本 (scripts/build.py) with GCC..."

# 调用 Python 脚本
python scripts/build.py install "$@"

echo "--- Python 脚本执行完毕。"
