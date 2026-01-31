#!/bin/bash

# 获取脚本所在目录
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# 调用 python 构建脚本，并传入 --no-opt 参数
python3 "$SCRIPT_DIR/build.py" --no-opt --no-tidy "$@"
