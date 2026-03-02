#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../../../.." && pwd)"
TT_WINDOWS_CLI_BACKEND=rust \
TT_RUST_RUNTIME_SYNC_STRICT=1 \
python "${REPO_ROOT}/scripts/run.py" \
  build \
  --app tracer_windows_rust_cli \
  --profile release_bundle \
  --build-dir build \
  "$@"
