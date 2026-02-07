#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# 调用 build.py auto 命令 (分析流程，不包含自动修复)
python "$SCRIPT_DIR/../../build.py" auto "$@"
