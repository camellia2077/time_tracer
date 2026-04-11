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
3. Pipeline-owned TXT day-block semantics are also exposed to hosts through the
   dedicated `tracer_core_runtime_txt_json` runtime family.

## Capability / Ownership Index

1. `pipeline` owns month-TXT authoring semantics, including TXT day-block
   normalization, extraction, replacement, and default `MMDD` resolution.
2. `tracer_core` owns the machine-readable DTO semantics for TXT runtime
   actions; the current public action family is `default_day_marker`,
   `resolve_day_block`, and `replace_day_block`.
3. `tracer_transport` owns JSON envelope/codec mechanics, not TXT business
   meaning.
4. `tracer_core_bridge_common` owns shared bridge helpers only; it is not the
   owner of TXT runtime actions or day-block rules.
5. `tracer_adapters_io` may read/write month TXT files, but it does not own
   day-block parsing, validation, or replacement semantics.

## TXT Day-Block Docs

Open these when the task changes shared TXT day-block behavior or runtime-facing
contracts:

1. [Detailed library doc](../../docs/time_tracer/architecture/libraries/tracer_core.md)
2. [TXT runtime JSON contract](../../docs/time_tracer/core/contracts/text/runtime_txt_day_block_json_contract_v1.md)
3. [Core C ABI contract](../../docs/time_tracer/core/shared/c_abi.md)

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

## Tests / Semantics Covered

1. `libs/tracer_core/tests/**` 主要保护 core 业务语义与 capability DTO 边界。
2. reporting 语义回归会区分“空窗口成功”与“target 不存在失败”。
3. 详细测试意图见
   [docs/time_tracer/architecture/libraries/tracer_core.md](../../docs/time_tracer/architecture/libraries/tracer_core.md).

## Validate

```powershell
python tools/run.py validate --plan tools/toolchain/config/validate/tracer_core_capabilities.toml --paths-file tools/toolchain/config/validate/pipeline.paths
python tools/run.py verify --app tracer_core_shell --profile fast --concise
```
