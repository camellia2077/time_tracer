# tracer_core

## Purpose

Local entrypoint for agents touching the core business-logic library.

## When To Open

- Open this when the task touches `libs/tracer_core`.
- Use it to jump into the detailed library doc and the cross-library map.

## What This Doc Does Not Cover

- Internal layer routing inside the library
- Detailed boundary rules
- Full validation strategy

## Open Next

1. [Detailed library doc](../../docs/time_tracer/architecture/libraries/tracer_core.md)
2. [Library dependency map](../../docs/time_tracer/architecture/library_dependency_map.md)
3. [Core agent onboarding](../../docs/time_tracer/core/specs/AGENT_ONBOARDING.md)
4. [Capability dependency map](../../docs/time_tracer/core/architecture/tracer_core_capability_dependency_map.md)
5. [Capability boundary contract](../../docs/time_tracer/core/design/tracer_core_capability_boundary_contract.md)

## Runtime Surface

1. Aggregate application runtime surface:
   - `ITracerCoreRuntime`
   - `TracerCoreRuntime` only aggregates prebuilt capability APIs; host/runtime
     wiring should stay outside this class
   - shell/runtime bridge helpers now live under `src/application/runtime_bridge/**`,
     not under capability-owned `application/ports/**`
2. Capability entry points:
   - `runtime.pipeline()`
   - `runtime.query()`
   - `runtime.report()`
   - `runtime.tracer_exchange()`

## Exchange Docs

Open these when the task is specifically about `tracer exchange` or
`infra/crypto`:

1. [Capability dependency map (exchange owner/deps)](../../docs/time_tracer/core/architecture/tracer_core_capability_dependency_map.md)
2. [Capability boundary contract (exchange owner paths/public surfaces)](../../docs/time_tracer/core/design/tracer_core_capability_boundary_contract.md)
3. [Tracer exchange package contract (`TTPKG v3`)](../../docs/time_tracer/core/contracts/crypto/tracer_exchange_package_v3.md)
4. [Runtime crypto JSON contract](../../docs/time_tracer/core/contracts/crypto/runtime_crypto_json_contract_v1.md)

## Validation Docs

Open these when the task is specifically about converter-config validation,
TXT structure validation, TXT logic validation, or ingest validation order:

1. [Ingest persistence boundary](../../docs/time_tracer/core/design/ingest-persistence-boundary.md)
2. [Validation error codes](../../docs/time_tracer/core/errors/error-codes.md)
3. [Core architecture index](../../docs/time_tracer/core/architecture/README.md)

## Validate

```powershell
python tools/run.py validate --plan tools/toolchain/config/validate/tracer_core_capabilities.toml --paths-file tools/toolchain/config/validate/pipeline.paths
python tools/run.py verify --app tracer_core_shell --profile fast --concise
```
