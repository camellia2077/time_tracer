#!/bin/bash

# Run clang-tidy only and generate build_tidy logs/tasks.
# User preference: keep-going is enabled for tidy runs.

set -euo pipefail

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
REPO_ROOT="$( cd "$SCRIPT_DIR/../../../.." && pwd )"
RUN_PY="$REPO_ROOT/scripts/run.py"

if [[ ! -f "$RUN_PY" ]]; then
  echo "Error: scripts/run.py not found: $RUN_PY" >&2
  exit 1
fi

python "$RUN_PY" tidy --app time_tracer --keep-going "$@"
