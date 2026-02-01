#!/bin/bash

set -e

cd "$(dirname "$0")/.."
echo "--- 已切换到项目目录: $(pwd)"

echo "--- 正在执行 Python 构建脚本 (scripts/build.py)"
python scripts/build.py "$@"

echo "--- Python 脚本执行完毕。"
