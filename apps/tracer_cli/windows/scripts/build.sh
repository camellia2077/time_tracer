#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../../.." && pwd)"
RUN_PY="${REPO_ROOT}/scripts/run.py"

print_usage() {
  echo "Usage: $0 [--runpy-args <run.py build args...>] [--build-args <cmake --build args...>]"
  echo ""
  echo "Channels:"
  echo "  --runpy-args   Forward following args to scripts/run.py build."
  echo "  --build-args   Forward following args to cmake --build (default channel)."
  echo ""
  echo "Examples:"
  echo "  $0 --target time_tracer_cli"
  echo "  $0 --runpy-args --cmake-args=-DENABLE_LTO=ON --build-args --target time_tracer_cli"
}

if [[ ! -f "${RUN_PY}" ]]; then
  echo "Error: scripts/run.py not found: ${RUN_PY}" >&2
  exit 1
fi

RUN_PY_ARGS=()
BUILD_ARGS=()
ARG_MODE="build"  # Backward compatibility: args default to cmake --build channel.
while [[ $# -gt 0 ]]; do
  case "$1" in
    -h|--help)
      print_usage
      exit 0
      ;;
    --runpy-args)
      ARG_MODE="runpy"
      shift
      continue
      ;;
    --build-args)
      ARG_MODE="build"
      shift
      continue
      ;;
  esac

  if [[ "${ARG_MODE}" == "runpy" ]]; then
    RUN_PY_ARGS+=("$1")
  else
    BUILD_ARGS+=("$1")
  fi
  shift
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

echo "[tracer_windows_cli build] profile=release_safe"
"${PYTHON_BIN[@]}" "${RUN_PY}" build \
  --app tracer_windows_cli \
  --profile release_safe \
  --cmake-args=-DENABLE_CLANG_TIDY=OFF \
  "${RUN_PY_ARGS[@]}" \
  -- "${BUILD_ARGS[@]}"
