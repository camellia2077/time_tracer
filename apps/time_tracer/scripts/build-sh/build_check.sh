#!/bin/bash

# Release optimized build with static checks (clang-tidy ON).
# LTO is enabled only for report DLL targets.

set -euo pipefail

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
# Repo root is 4 levels up: build-sh -> scripts -> app -> apps -> repo_root
REPO_ROOT="$( cd "$SCRIPT_DIR/../../../.." && pwd )"
RUN_PY="$REPO_ROOT/scripts/run.py"
BUILD_DIR="${BUILD_DIR:-build_check}"

if [[ ! -f "$RUN_PY" ]]; then
  echo "Error: scripts/run.py not found: $RUN_PY" >&2
  exit 1
fi

DLL_TARGETS=(
  reports_shared
  DayMdFormatter DayTexFormatter DayTypFormatter
  MonthMdFormatter MonthTexFormatter MonthTypFormatter
  RangeMdFormatter RangeTexFormatter RangeTypFormatter
)

# Phase 1:
# Build the whole app with LTO disabled to avoid known toolchain issues
# on executable linking.
python "$RUN_PY" configure --app time_tracer --tidy --build-dir "$BUILD_DIR" -- \
  -DCMAKE_BUILD_TYPE=Release \
  -DDISABLE_OPTIMIZATION=OFF \
  -DENABLE_LTO=OFF \
  -DWARNING_LEVEL=2
python "$RUN_PY" build --app time_tracer --tidy --build-dir "$BUILD_DIR" -- "$@"

# Phase 2:
# Reconfigure with LTO and rebuild only report DLL targets.
python "$RUN_PY" configure --app time_tracer --tidy --build-dir "$BUILD_DIR" -- \
  -DCMAKE_BUILD_TYPE=Release \
  -DDISABLE_OPTIMIZATION=OFF \
  -DENABLE_LTO=ON \
  -DWARNING_LEVEL=2 \
  -DCMAKE_EXE_LINKER_FLAGS=-fuse-ld=lld \
  -DCMAKE_SHARED_LINKER_FLAGS=-fuse-ld=lld
python "$RUN_PY" build --app time_tracer --tidy --build-dir "$BUILD_DIR" -- --target "${DLL_TARGETS[@]}"
