#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../../../.." && pwd)"
exec python "${REPO_ROOT}/scripts/run.py" \
  build \
  --app tracer_core \
  --profile release_bundle \
  --build-dir build \
  -- \
  --target tc_rpt_shared_lib \
  --target tc_shared_dll \
  "$@"
