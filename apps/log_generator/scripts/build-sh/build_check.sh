#!/bin/bash

# Release optimized build with static checks (clang-tidy ON).
# Main entry (preferred): build_check

set -euo pipefail

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
# Repo root is 4 levels up: build-sh -> scripts -> app -> apps -> repo_root
REPO_ROOT="$( cd "$SCRIPT_DIR/../../../.." && pwd )"
RUN_PY="$REPO_ROOT/scripts/run.py"

if [[ ! -f "$RUN_PY" ]]; then
  echo "Error: scripts/run.py not found: $RUN_PY" >&2
  exit 1
fi

# De-hardcoded defaults (can be overridden by env vars):
#   BUILD_DIR=build_check
#   BUILD_TYPE=Release
#   DISABLE_OPT=OFF
#   ENABLE_LTO=OFF
#   WARNING_LEVEL=2
BUILD_DIR="${BUILD_DIR:-build_check}"
BUILD_TYPE="${BUILD_TYPE:-Release}"
DISABLE_OPT="${DISABLE_OPT:-OFF}"
ENABLE_LTO="${ENABLE_LTO:-OFF}"
WARNING_LEVEL="${WARNING_LEVEL:-2}"

echo "--- Forwarding to unified runner: scripts/run.py configure/build (Release Optimized, check ON) ---"
echo "--- Profile: build_dir=$BUILD_DIR, build_type=$BUILD_TYPE, disable_opt=$DISABLE_OPT, lto=$ENABLE_LTO, warning_level=$WARNING_LEVEL ---"
# NOTE:
# Main executable LTO is intentionally disabled by default because of
# known toolchain instability for this target.
# DLL-only LTO can be enabled in projects that actually build DLL targets.
python "$RUN_PY" configure --app log_generator --tidy --build-dir "$BUILD_DIR" -- \
  -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
  -DDISABLE_OPTIMIZATION="$DISABLE_OPT" \
  -DENABLE_LTO="$ENABLE_LTO" \
  -DWARNING_LEVEL="$WARNING_LEVEL"
python "$RUN_PY" build --app log_generator --tidy --build-dir "$BUILD_DIR" -- "$@"

