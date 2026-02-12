#!/bin/bash

# Re-split existing build_tidy/build.log into task logs.
# Uses `scripts/config.toml` tidy settings by default:
#   max_lines, max_diags, batch_size
# Optional overrides:
#   --max-lines N --max-diags N --batch-size N --parse-workers N

set -euo pipefail

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
REPO_ROOT="$( cd "$SCRIPT_DIR/../../../.." && pwd )"
RUN_PY="$REPO_ROOT/scripts/run.py"

if [[ ! -f "$RUN_PY" ]]; then
  echo "Error: scripts/run.py not found: $RUN_PY" >&2
  exit 1
fi

python "$RUN_PY" tidy-split --app log_generator "$@"
