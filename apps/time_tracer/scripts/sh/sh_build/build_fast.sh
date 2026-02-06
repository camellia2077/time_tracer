#!/bin/bash

# 获取脚本所在目录
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# 调用 Python 脚本 (位于 ../../build.py)
python "$SCRIPT_DIR/../../build.py" --build-dir build_fast --no-opt --no-lto --no-tidy --no-warn "$@"
