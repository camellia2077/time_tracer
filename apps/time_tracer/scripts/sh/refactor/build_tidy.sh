#!/bin/bash

set -euo pipefail

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_DIR="$SCRIPT_DIR/../../.."
BUILD_DIR="$PROJECT_DIR/build_tidy"
LOG_FILE="$BUILD_DIR/build.log"
TASKS_DIR="$BUILD_DIR/tasks"

mkdir -p "$BUILD_DIR"

set +e
python "$SCRIPT_DIR/../../build.py" --build-dir build_tidy --no-pch --tidy --fail-fast --analyze-tidy "$@" 2>&1 | tee "$LOG_FILE"
status=${PIPESTATUS[0]}
set -e

python "$SCRIPT_DIR/../../workflow.py" split --log "$LOG_FILE" --out "$TASKS_DIR"
python "$SCRIPT_DIR/../../workflow.py" summary --dir "$TASKS_DIR"

exit $status
