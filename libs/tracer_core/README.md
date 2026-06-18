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
4. Reporting now exposes one canonical single-request ABI surface:
   - `tracer_core_runtime_temporal_report_json`
   - `report_batch` remains a separate helper for multi-days recent text
     rendering only.

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

## Current Authored TXT Event Semantics

1. Current ingest accepts two authored event forms:
   - point event: `HHMMtoken`
   - interval event: `HHMM-HHMMtoken`
2. The authored event model is represented as `RawEvent`, which keeps authored
   facts before canonical activity mapping and record materialization:
   - point event stores only the authored end boundary
   - interval event stores explicit authored start/end boundaries
3. Current semantic meaning:
   - point event means "this activity ends at `HHMM`"
   - interval event means "this activity explicitly happened from `start` to `end`"
   - activity token parsing and canonical mapping stay separate from time
     semantics
4. Current mixed-timeline rules:
   - conversion advances by the last known boundary
   - point event materializes `last_known_boundary -> point.end`
   - interval event materializes `interval.start -> interval.end`
   - after either event, the next boundary becomes the current event end
5. Current validation rules:
   - gaps are allowed and remain unrecorded
   - overlap is invalid
   - interval must satisfy `start < end` after relative-boundary expansion
   - wrapped cross-midnight interval chains are allowed when they remain
     monotonic after expansion
   - short backward interval ranges are rejected
   - wake keyword remains point-event-only and must be the first semantic event
6. This is a current ingest and validation semantic model, not a claim that the
   entire ingest main flow is "a weighted tree". Tree-style aggregation belongs
   to downstream statistics/query semantics after records have already been
   materialized.

## Interval Support Implementation Notes

1. Parser stage:
   - `TextParser` recognizes both `HHMMtoken` and `HHMM-HHMMtoken`
   - inline remark extraction stays shared across both forms
2. Validation stage:
   - TXT line validation accepts both point and interval syntax
   - structure/logic validation owns overlap, invalid-range, wake-position, and
     mixed-timeline checks
3. Converter stage:
   - point events still derive duration from the last known boundary
   - interval events use authored start/end directly
   - continuation-day behavior remains supported; an explicit first interval may
     leave a gap from the previous-day boundary
4. Persistence stage:
   - processed records remain standard `start/end/duration/project_path/remark`
     records
   - interval support in stage 1 does not require a DB schema change

## Interval Test Intent

1. Parser / line-validation tests should cover both point and interval syntax.
2. Structure tests should cover:
   - wake as first point event only
   - point after interval uses the interval end as the next boundary
   - legal gaps
   - illegal overlaps
   - explicit invalid backward intervals
   - wrapped cross-midnight interval chains
3. Converter tests should cover:
   - pure point timelines stay unchanged
   - pure interval timelines preserve authored start/end
   - mixed point/interval timelines advance by interval end
   - cross-midnight intervals preserve wrapped duration
   - continuation-day first-interval gaps stay unfilled

## Tests / Semantics Covered

1. `libs/tracer_core/tests/**` 主要保护 core 业务语义与 capability DTO 边界。
2. reporting 语义回归会区分“empty recent/range window 成功”与
   “day/week/month/year target 不存在失败”。
3. temporal reporting contract 还会覆盖 `recent_days + optional anchor_date`
   的固定窗口解析与 recent 语义保留。
4. 详细测试意图见
   [docs/time_tracer/architecture/libraries/tracer_core.md](../../docs/time_tracer/architecture/libraries/tracer_core.md).

## Test Asset Boundary

1. core 语义测试源码继续放在 `libs/tracer_core/tests/**`，不迁入 `test/**`。
2. `test/data/**` 是跨 CLI / shell / Android 复用的 canonical TXT 输入。
3. `test/fixtures/text/**` 与 `test/fixtures/config/**` 适合承载 core 相关的
   小型专项样本，例如最小合法月文件、非法 TXT、legacy config 与 custom
   config。
4. `test/golden/**` 只承载最终输出对账基线，不承载 core 中间态数据。
5. 本项目的数据真源仍是文本输入，不引入 checked-in 的 DB 中间态目录。

## Validate

```powershell
python tools/run.py validate --plan tools/toolchain/config/validate/tracer_core_capabilities.toml --paths-file tools/toolchain/config/validate/pipeline.paths
python tools/run.py verify --app tracer_core_shell --profile fast --concise
```
