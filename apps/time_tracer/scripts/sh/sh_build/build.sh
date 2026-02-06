#!/bin/bash

# 如果任何命令执行失败，立即退出脚本
set -e

# 切换到项目根目录 (scripts/sh/build/ -> scripts/sh/ -> scripts/ -> project_root)
cd "$(dirname "$0")/../../.."
echo "--- 已切换到项目根目录: $(pwd)"

echo "--- 正在执行 Python 构建脚本 (scripts/build.py)"

# 调用 Python 脚本并指定构建目录
python scripts/build.py --build-dir build "$@"

echo "--- Python 脚本执行完毕。"
