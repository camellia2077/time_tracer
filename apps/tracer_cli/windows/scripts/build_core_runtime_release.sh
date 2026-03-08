#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../../../.." && pwd)"

if command -v python >/dev/null 2>&1; then
  PYTHON_BIN=(python)
elif command -v py >/dev/null 2>&1; then
  PYTHON_BIN=(py -3)
elif command -v python3 >/dev/null 2>&1; then
  PYTHON_BIN=(python3)
else
  echo "Error: python runtime not found (python/python3/py)." >&2
  exit 127
fi

exec "${PYTHON_BIN[@]}" "${REPO_ROOT}/tools/run.py" \
  build \
  --app tracer_core \
  --profile release_bundle \
  --build-dir build \
  -- \
  --target tc_rpt_shared_lib \
  --target tc_shared_dll \
  "$@"
