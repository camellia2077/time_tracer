#!/bin/bash

# Re-split existing build_tidy/build.log into task logs.
# Uses `scripts/config.toml` tidy settings by default:
#   max_lines, max_diags, batch_size
# Optional overrides:
#   --max-lines N --max-diags N --batch-size N --parse-workers N
#
# Script-level override variables (optional):
#   TT_TIDY_SPLIT_MAX_LINES
#   TT_TIDY_SPLIT_MAX_DIAGS
#   TT_TIDY_SPLIT_BATCH_SIZE
#   TT_TIDY_SPLIT_PARSE_WORKERS
#
# Priority:
#   CLI args passed to this script > script/env overrides > config.toml

set -euo pipefail

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
REPO_ROOT="$( cd "$SCRIPT_DIR/../../../.." && pwd )"
RUN_PY="$REPO_ROOT/scripts/run.py"

if [[ ! -f "$RUN_PY" ]]; then
  echo "Error: scripts/run.py not found: $RUN_PY" >&2
  exit 1
fi

TT_TIDY_SPLIT_MAX_LINES="${TT_TIDY_SPLIT_MAX_LINES:-}"
TT_TIDY_SPLIT_MAX_DIAGS="${TT_TIDY_SPLIT_MAX_DIAGS:-}"
TT_TIDY_SPLIT_BATCH_SIZE="${TT_TIDY_SPLIT_BATCH_SIZE:-}"
TT_TIDY_SPLIT_PARSE_WORKERS="${TT_TIDY_SPLIT_PARSE_WORKERS:-}"

EXTRA_ARGS=()

if [[ -n "$TT_TIDY_SPLIT_MAX_LINES" ]]; then
  EXTRA_ARGS+=(--max-lines "$TT_TIDY_SPLIT_MAX_LINES")
fi
if [[ -n "$TT_TIDY_SPLIT_MAX_DIAGS" ]]; then
  EXTRA_ARGS+=(--max-diags "$TT_TIDY_SPLIT_MAX_DIAGS")
fi
if [[ -n "$TT_TIDY_SPLIT_BATCH_SIZE" ]]; then
  EXTRA_ARGS+=(--batch-size "$TT_TIDY_SPLIT_BATCH_SIZE")
fi
if [[ -n "$TT_TIDY_SPLIT_PARSE_WORKERS" ]]; then
  EXTRA_ARGS+=(--parse-workers "$TT_TIDY_SPLIT_PARSE_WORKERS")
fi

python "$RUN_PY" tidy-split --app time_tracer "${EXTRA_ARGS[@]}" "$@"
