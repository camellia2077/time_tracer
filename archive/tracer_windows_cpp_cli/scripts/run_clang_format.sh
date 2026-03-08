#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../../../../.." && pwd)"

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

MODE="${1:-check}"   # check | fix
SCOPE="${2:-all}"    # all | core | cli
BUILD_DIR="${3:-build_fast}"

TARGET=""
case "${MODE}:${SCOPE}" in
  check:all) TARGET="check_format_all" ;;
  check:core) TARGET="check_format_core" ;;
  check:cli) TARGET="check_format_cli" ;;
  fix:all) TARGET="format_all" ;;
  fix:core) TARGET="format_core" ;;
  fix:cli) TARGET="format_cli" ;;
  *)
    echo "Usage: $0 [check|fix] [all|core|cli] [build_dir]"
    echo "Example: $0 check all build_fast"
    exit 2
    ;;
esac

echo "[clang-format] mode=${MODE} scope=${SCOPE} build_dir=${BUILD_DIR} target=${TARGET}"
"${PYTHON_BIN[@]}" "${REPO_ROOT}/tools/run.py" configure --app tracer_windows_cli --build-dir "${BUILD_DIR}"
"${PYTHON_BIN[@]}" "${REPO_ROOT}/tools/run.py" build --app tracer_windows_cli --build-dir "${BUILD_DIR}" -- --target "${TARGET}" -j
