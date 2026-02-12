#!/bin/bash

# Release optimized build without clang-tidy.

set -euo pipefail

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_DIR="$( cd "$SCRIPT_DIR/../.." && pwd )"
REPO_ROOT="$( cd "$PROJECT_DIR/../.." && pwd )"
RUN_PY="$REPO_ROOT/scripts/run.py"

if [[ ! -f "$RUN_PY" ]]; then
  echo "Error: scripts/run.py not found: $RUN_PY" >&2
  exit 1
fi

echo "--- Forwarding to unified runner: scripts/run.py configure/build (Release Optimized, clang-tidy OFF) ---"

python "$RUN_PY" configure --app log_generator -- \
  -DCMAKE_BUILD_TYPE=Release \
  -DDISABLE_OPTIMIZATION=OFF \
  -DENABLE_LTO=OFF \
  -DWARNING_LEVEL=2
python "$RUN_PY" build --app log_generator -- "$@"
