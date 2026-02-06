#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# 调用 build.py (位于 ../../build.py)
python "$SCRIPT_DIR/../../build.py" --build-dir build_tidy --no-pch --tidy --fail-fast --analyze-tidy --split-tasks "$@"
