#!/bin/bash

# Fast profile (build directory: build_fast).

set -euo pipefail

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
# Repo root is 4 levels up: build-sh -> scripts -> app -> apps -> repo_root
REPO_ROOT="$( cd "$SCRIPT_DIR/../../../.." && pwd )"
RUN_PY="$REPO_ROOT/scripts/run.py"

if [[ ! -f "$RUN_PY" ]]; then
  echo "Error: scripts/run.py not found: $RUN_PY" >&2
  exit 1
fi

echo "--- Forwarding to unified runner: scripts/run.py configure/build (Fast Profile) ---"
python "$RUN_PY" configure --app time_tracer --build-dir build_fast -- \
  -DCMAKE_BUILD_TYPE=Debug \
  -DDISABLE_OPTIMIZATION=ON \
  -DENABLE_LTO=OFF \
  -DWARNING_LEVEL=0
python "$RUN_PY" build --app time_tracer --build-dir build_fast -- "$@"
