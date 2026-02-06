#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# 调用 refactor.py auto 命令（位于 ../../refactor.py）
python "$SCRIPT_DIR/../../build.py" auto "$@"
