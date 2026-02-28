# tracer_core

Core runtime implementation for TimeTracer (`domain + application + infrastructure + api`).

## Scope Policy

- This file is a short execution entry. Open docs only as needed.
- For localized tasks, prefer direct search (`rg`) and scoped file reads.

## Architecture Docs

- Onboarding: `docs/time_tracer/core/specs/AGENT_ONBOARDING.md`
- Core docs index: `docs/time_tracer/core/README.md`
- Architecture index: `docs/time_tracer/core/architecture/README.md`
- Contracts index: `docs/time_tracer/core/contracts/README.md`
- C ABI: `docs/time_tracer/core/contracts/c_abi.md`
- Stats contracts: `docs/time_tracer/core/contracts/stats/README.md`

## Scan Scope (Ignore Generated Artifacts)

Prioritize:

- `apps/tracer_core/src`
- `apps/tracer_core/config`
- `docs/time_tracer/core`

Exclude generated outputs by default:

- `apps/tracer_core/build/**`
- `apps/tracer_core/build_fast/**`
- `apps/tracer_core/build_fast_*/**`
- `apps/tracer_core/.cache/**`
- `test/output/**`

Reference command:

```powershell
rg -n --glob '!**/build/**' --glob '!**/build_fast/**' --glob '!**/build_fast_*/**' --glob '!**/.cache/**' "PATTERN" apps/tracer_core docs/time_tracer/core
```

## Build / Verify (Python Entry, Recommended)

Run from repo root:

```powershell
python scripts/run.py build --app tracer_core --profile fast
python scripts/run.py verify --app tracer_core --profile fast --concise
```

## Change Guardrails

1. For ABI/contract changes, update `docs/time_tracer/core/contracts/*` first.
2. For cross-app fields (CLI/Android), confirm downstream consumers are updated.
3. Keep business rules in `domain/application`; do not leak platform details back into core layers.
