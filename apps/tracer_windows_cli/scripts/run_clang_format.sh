#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../../.." && pwd)"

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
python "${REPO_ROOT}/scripts/run.py" configure --app tracer_windows_cli --build-dir "${BUILD_DIR}"
python "${REPO_ROOT}/scripts/run.py" build --app tracer_windows_cli --build-dir "${BUILD_DIR}" -- --target "${TARGET}" -j
