#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../../../../.." && pwd)"
RUN_PY="${REPO_ROOT}/scripts/run.py"

MODE="${1:-check}" # check | fix
shift || true

print_usage() {
  echo "Usage: $0 [check|fix] [--runpy-args <format args...>]"
  echo "Examples:"
  echo "  $0 check"
  echo "  $0 fix"
  echo "  $0 check --runpy-args -- --all"
}

if [[ "${MODE}" == "-h" || "${MODE}" == "--help" ]]; then
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

case "${MODE}" in
  check)
    FORMAT_ARGS=(-- --check)
    ;;
  fix)
    FORMAT_ARGS=(--)
    ;;
  *)
    print_usage
    exit 2
    ;;
esac

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

echo "[rust fmt] mode=${MODE} app=tracer_windows_rust_cli"
"${PYTHON_BIN[@]}" "${RUN_PY}" format --app tracer_windows_rust_cli "${FORMAT_ARGS[@]}" "${RUN_PY_ARGS[@]}"
