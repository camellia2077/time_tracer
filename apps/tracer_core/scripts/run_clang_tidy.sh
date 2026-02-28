#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../../.." && pwd)"

print_usage() {
  echo "Usage: $0 [check|fix] [build_dir] [--keep-going|--no-keep-going]"
  echo "Examples:"
  echo "  $0 check build_tidy --keep-going"
  echo "  $0 fix build_tidy --keep-going"
}

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
  print_usage
  exit 0
fi

MODE="${1:-check}"       # check | fix
BUILD_DIR="${2:-build_tidy}"
KEEP_GOING="true"        # true | false

case "${MODE}" in
  check|fix) ;;
  *)
    echo "Error: invalid mode: ${MODE}" >&2
    print_usage
    exit 2
    ;;
esac

for ARG in "${@:3}"; do
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

echo "[time_tracer clang-tidy] mode=${MODE} build_dir=${BUILD_DIR} keep_going=${KEEP_GOING}"
"${PYTHON_BIN[@]}" "${REPO_ROOT}/scripts/run.py" configure --app tracer_core --build-dir "${BUILD_DIR}" --tidy

if [[ "${MODE}" == "check" ]]; then
  # Use `run.py tidy` to preserve build.log + tasks/batch_* splitting workflow.
  "${PYTHON_BIN[@]}" "${REPO_ROOT}/scripts/run.py" tidy --app tracer_core --build-dir "${BUILD_DIR}" "${KEEP_GOING_ARGS[@]}" -- --target tidy
else
  BUILD_EXTRA_ARGS=(--target tidy-fix)
  if [[ "${KEEP_GOING}" == "true" ]]; then
    BUILD_EXTRA_ARGS+=(-- -k 0)
  fi
  "${PYTHON_BIN[@]}" "${REPO_ROOT}/scripts/run.py" build --app tracer_core --build-dir "${BUILD_DIR}" --tidy -- "${BUILD_EXTRA_ARGS[@]}"
fi

