#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

# 调用 tidy.py (位于 ../../tidy.py)
python "$SCRIPT_DIR/../../build.py" "$@"
