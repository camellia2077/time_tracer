#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

"${SCRIPT_DIR}/build_core_runtime_release.sh" "$@"
"${SCRIPT_DIR}/build_rust_from_windows_build.sh" "$@"
