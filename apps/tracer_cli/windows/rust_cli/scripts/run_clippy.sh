#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../../../../.." && pwd)"
RUN_PY="${REPO_ROOT}/scripts/run.py"

print_usage() {
  echo "Usage: $0 [--runpy-args <lint args...>]"
  echo "Examples:"
  echo "  $0"
  echo "  $0 --runpy-args -- -D warnings"
}

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
  print_usage
  exit 0
fi

if [[ ! -f "${RUN_PY}" ]]; then
  echo "Error: scripts/run.py not found: ${RUN_PY}" >&2
  exit 1
fi

RUN_PY_ARGS=()
if [[ $# -gt 0 ]]; then
  if [[ "$1" == "--runpy-args" ]]; then
    shift
  fi
  RUN_PY_ARGS=("$@")
fi

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

echo "[rust clippy] app=tracer_windows_rust_cli"
"${PYTHON_BIN[@]}" "${RUN_PY}" lint --app tracer_windows_rust_cli -- "${RUN_PY_ARGS[@]}"
