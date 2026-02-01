#!/bin/bash

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

BUILD_DIR_NAME="build_debug"
export TIME_TRACER_BUILD_DIR="$BUILD_DIR_NAME"

python3 "$SCRIPT_DIR/build.py" "$@"
cmake --build "$SCRIPT_DIR/../$BUILD_DIR_NAME" --target tidy-fix
