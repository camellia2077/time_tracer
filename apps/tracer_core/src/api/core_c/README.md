# Core C ABI (Local Guide)

This folder contains the implementation and tests for the core C ABI boundary.

Canonical ABI spec:
- `docs/time_tracer/core/contracts/c_abi.md`

Local rules:
1. Exported symbols must use `tracer_core_*` prefix only.
2. Do not add or reintroduce `tt_*` symbols.
3. Keep ABI requests/responses as UTF-8 JSON strings unless a planned ABI version change is approved.
4. Core C ABI runtime signaling should be callback-based (`tracer_core_set_log_callback` / `tracer_core_set_diagnostics_callback` / `tracer_core_set_crypto_progress_callback`) instead of direct console output assumptions.

When changing ABI:
1. Update this folder code/tests.
2. Update `docs/time_tracer/core/contracts/c_abi.md`.
3. Run verification via `python scripts/run.py verify --app tracer_core --build-dir build_fast --concise`.
