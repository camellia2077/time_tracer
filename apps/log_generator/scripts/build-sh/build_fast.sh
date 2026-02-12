#!/bin/bash

# Fast profile (build directory: build_fast).

set -euo pipefail

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_DIR="$( cd "$SCRIPT_DIR/../.." && pwd )"
REPO_ROOT="$( cd "$PROJECT_DIR/../.." && pwd )"
RUN_PY="$REPO_ROOT/scripts/run.py"

if [[ ! -f "$RUN_PY" ]]; then
  echo "Error: scripts/run.py not found: $RUN_PY" >&2
  exit 1
fi

echo "--- Forwarding to unified runner: scripts/run.py configure/build (Fast Profile) ---"
python "$RUN_PY" configure --app log_generator --build-dir build_fast -- \
  -DCMAKE_BUILD_TYPE=Debug \
  -DDISABLE_OPTIMIZATION=ON \
  -DENABLE_LTO=OFF \
  -DWARNING_LEVEL=0
python "$RUN_PY" build --app log_generator --build-dir build_fast -- "$@"
