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

TT_WINDOWS_CLI_BACKEND=rust \
TT_RUST_RUNTIME_SYNC_STRICT=1 \
"${PYTHON_BIN[@]}" "${REPO_ROOT}/tools/run.py" \
  build \
  --app tracer_windows_rust_cli \
  --profile release_bundle \
  --build-dir build \
  "$@"
