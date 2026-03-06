#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../../.." && pwd)"

print_usage() {
  echo "Usage: $0 [build_dir] [tidy-split options...]"
  echo "Example: $0 build_tidy --max-lines 120 --max-diags 8 --batch-size 15"
}

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
  print_usage
  exit 0
fi

BUILD_DIR="build_tidy"
EXTRA_ARGS=()
if [[ $# -gt 0 ]]; then
  if [[ "${1}" != -* ]]; then
    BUILD_DIR="${1}"
    shift
  fi
  EXTRA_ARGS=("$@")
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

echo "[time_tracer clang-tidy split-only] build_dir=${BUILD_DIR}"
"${PYTHON_BIN[@]}" "${REPO_ROOT}/scripts/run.py" tidy-split --app tracer_core --build-dir "${BUILD_DIR}" "${EXTRA_ARGS[@]}"

