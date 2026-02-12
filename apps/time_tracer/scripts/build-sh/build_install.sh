#!/bin/bash

# Install profile.

set -euo pipefail

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
# Repo root is 4 levels up: build-sh -> scripts -> app -> apps -> repo_root
REPO_ROOT="$( cd "$SCRIPT_DIR/../../../.." && pwd )"
RUN_PY="$REPO_ROOT/scripts/run.py"

if [[ ! -f "$RUN_PY" ]]; then
  echo "Error: scripts/run.py not found: $RUN_PY" >&2
  exit 1
fi

echo "--- Forwarding to unified runner: scripts/run.py build (Install Target) ---"
python "$RUN_PY" build --app time_tracer -- --target install "$@"
