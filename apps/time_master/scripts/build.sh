#!/bin/bash

# 如果任何命令执行失败，立即退出脚本
set -e

# 切换到脚本所在的目录 (scripts)，然后再返回上一级到项目根目录
cd "$(dirname "$0")/.."
echo "--- 已切换到项目根目录: $(pwd)"

echo "--- 正在执行 Python 构建脚本 (scripts/build.py)"

# 使用新的相对路径来调用 Python 脚本
python scripts/build.py "$@"

echo "--- Python 脚本执行完毕。"