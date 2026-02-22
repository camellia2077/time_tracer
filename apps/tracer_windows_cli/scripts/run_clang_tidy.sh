#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../../.." && pwd)"

print_usage() {
  echo "Usage: $0 [check|fix] [all|core|cli] [build_dir] [--keep-going|--no-keep-going]"
  echo "Examples:"
  echo "  $0 check all build_tidy --keep-going"
  echo "  $0 check all build_tidy --no-keep-going"
}

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
  print_usage
  exit 0
fi

MODE="${1:-check}"       # check | fix
SCOPE="${2:-all}"        # all | core | cli
BUILD_DIR="${3:-build_tidy}"
KEEP_GOING="true"        # true | false (check mode only)

for ARG in "${@:4}"; do
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

TARGET=""
case "${MODE}:${SCOPE}" in
  check:all) TARGET="tidy_all" ;;
  check:core) TARGET="tidy_core" ;;
  check:cli) TARGET="tidy_cli" ;;
  fix:all) TARGET="tidy_fix_all" ;;
  fix:core) TARGET="tidy_fix_core" ;;
  fix:cli) TARGET="tidy_fix_cli" ;;
  *)
    print_usage
    exit 2
    ;;
esac

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

KEEP_GOING_ARGS=()
if [[ "${KEEP_GOING}" == "true" ]]; then
  KEEP_GOING_ARGS=(--keep-going)
else
  KEEP_GOING_ARGS=(--no-keep-going)
fi

echo "[clang-tidy] mode=${MODE} scope=${SCOPE} build_dir=${BUILD_DIR} target=${TARGET} keep_going=${KEEP_GOING}"
"${PYTHON_BIN[@]}" "${REPO_ROOT}/scripts/run.py" configure --app tracer_windows_cli --build-dir "${BUILD_DIR}" --tidy

if [[ "${MODE}" == "check" ]]; then
  # Use `run.py tidy` to preserve build.log + tasks/batch_* splitting workflow.
  "${PYTHON_BIN[@]}" "${REPO_ROOT}/scripts/run.py" tidy --app tracer_windows_cli --build-dir "${BUILD_DIR}" "${KEEP_GOING_ARGS[@]}" -- --target "${TARGET}"
else
  if [[ "${KEEP_GOING}" == "false" ]]; then
    echo "[clang-tidy] note: fix mode currently ignores keep-going switch in this wrapper."
  fi
  "${PYTHON_BIN[@]}" "${REPO_ROOT}/scripts/run.py" build --app tracer_windows_cli --build-dir "${BUILD_DIR}" --tidy -- --target "${TARGET}" -j
fi
