#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../../.." && pwd)"
RUN_PY="${REPO_ROOT}/tools/run.py"

print_usage() {
  echo "Usage: $0 [check|fix] [build_dir] [--keep-going|--no-keep-going]"
  echo "Defaults:"
  echo "  source_scope=core_family"
  echo "  build_dir=build_tidy_core_family"
  echo "Examples:"
  echo "  $0 check"
  echo "  $0 check build_tidy_core_family --keep-going"
  echo "  $0 fix"
}

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
  print_usage
  exit 0
fi

MODE="check"
BUILD_DIR="build_tidy_core_family"
KEEP_GOING="true"

if [[ $# -gt 0 && "${1}" != -* ]]; then
  MODE="${1}"
  shift
fi

case "${MODE}" in
  check|fix) ;;
  *)
    echo "Error: invalid mode: ${MODE}" >&2
    print_usage
    exit 2
    ;;
esac

if [[ $# -gt 0 && "${1}" != -* ]]; then
  BUILD_DIR="${1}"
  shift
fi

for ARG in "$@"; do
  case "${ARG}" in
    --keep-going) KEEP_GOING="true" ;;
    --no-keep-going) KEEP_GOING="false" ;;
    -h|--help)
      print_usage
      exit 0
      ;;
    *)
      echo "Error: unknown argument: ${ARG}" >&2
      print_usage
      exit 2
      ;;
  esac
done

if [[ ! -f "${RUN_PY}" ]]; then
  echo "Error: tools/run.py not found: ${RUN_PY}" >&2
  exit 1
fi

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

KEEP_GOING_ARGS=()
if [[ "${KEEP_GOING}" == "true" ]]; then
  KEEP_GOING_ARGS=(--keep-going)
else
  KEEP_GOING_ARGS=(--no-keep-going)
fi

BASE_ARGS=(--app tracer_core_shell --source-scope core_family)

if [[ "${MODE}" == "check" ]]; then
  CMD=("${PYTHON_BIN[@]}" "${RUN_PY}" tidy "${BASE_ARGS[@]}" --build-dir "${BUILD_DIR}" "${KEEP_GOING_ARGS[@]}")
else
  CMD=("${PYTHON_BIN[@]}" "${RUN_PY}" tidy-fix "${BASE_ARGS[@]}" --tidy-build-dir "${BUILD_DIR}" "${KEEP_GOING_ARGS[@]}")
fi

echo "[libs-core clang-tidy] mode=${MODE} app=tracer_core_shell source_scope=core_family build_dir=${BUILD_DIR} keep_going=${KEEP_GOING}"
printf '[libs-core clang-tidy] command='
printf '%q ' "${CMD[@]}"
printf '\n'
"${CMD[@]}"
