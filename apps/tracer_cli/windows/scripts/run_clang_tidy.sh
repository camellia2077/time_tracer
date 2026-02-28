#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../../.." && pwd)"
RUN_PY="${REPO_ROOT}/scripts/run.py"

print_usage() {
  echo "Usage: $0 <batch_id> [--runpy-args <extra tidy-batch args...>]"
  echo "Examples:"
  echo "  $0 batch_002"
  echo "  $0 2"
  echo "  $0 batch_003 --runpy-args --full-every 1 --no-keep-going"
  echo ""
  echo "Default command:"
  echo "  python scripts/run.py tidy-batch --app tracer_windows_cli --batch-id <batch_id> \\"
  echo "    --strict-clean --run-verify --concise --full-every 3 --keep-going"
}

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
  print_usage
  exit 0
fi

if [[ ! -f "${RUN_PY}" ]]; then
  echo "Error: scripts/run.py not found: ${RUN_PY}" >&2
  exit 1
fi

if [[ $# -lt 1 ]]; then
  print_usage
  exit 2
fi

BATCH_ID="$1"
shift

RUN_PY_ARGS=()
if [[ $# -gt 0 ]]; then
  if [[ "$1" == "--runpy-args" ]]; then
    shift
  fi
  RUN_PY_ARGS=("$@")
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

echo "[clang-tidy batch] batch_id=${BATCH_ID}"
"${PYTHON_BIN[@]}" "${RUN_PY}" tidy-batch \
  --app tracer_windows_cli \
  --batch-id "${BATCH_ID}" \
  --strict-clean \
  --run-verify \
  --concise \
  --full-every 3 \
  --keep-going \
  "${RUN_PY_ARGS[@]}"
