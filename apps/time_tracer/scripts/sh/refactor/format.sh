#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# 项目根目录是 ../../../ (scripts/sh/tidy/ -> scripts/sh/ -> scripts/ -> project_root)
PROJECT_ROOT="$SCRIPT_DIR/../../.."

cmake -S "$PROJECT_ROOT" -B "$PROJECT_ROOT/build" -G Ninja
cmake --build "$PROJECT_ROOT/build" --target format
