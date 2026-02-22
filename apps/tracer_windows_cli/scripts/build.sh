#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../../.." && pwd)"
RUN_PY="${REPO_ROOT}/scripts/run.py"

if [[ ! -f "${RUN_PY}" ]]; then
  echo "Error: scripts/run.py not found: ${RUN_PY}" >&2
  exit 1
fi

# Resolve python launcher across shells:
# - MSYS/Git Bash may expose python3 but not python.
# - Windows may only provide py launcher.
if command -v python >/dev/null 2>&1; then
  PYTHON_BIN=(python)
elif command -v python3 >/dev/null 2>&1; then
  PYTHON_BIN=(python3)
elif command -v py >/dev/null 2>&1; then
  PYTHON_BIN=(py -3)
else
  echo "Error: python runtime not found (python/python3/py)." >&2
  exit 127
fi

echo "[tracer_windows_cli build] profile=release_safe"
"${PYTHON_BIN[@]}" "${RUN_PY}" build --app tracer_windows_cli --profile release_safe -- "$@"
