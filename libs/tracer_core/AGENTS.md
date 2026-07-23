# tracer_core Local Contract

## Scope

Applies to `libs/tracer_core/**`. This library owns shared domain and
application semantics used by every presentation and host.

## Required Read Set

Before changing core code, read:

1. `.agents/guides/docs/tracer_core-read-first.md`
2. `docs/time_tracer/architecture/libraries/tracer_core.md`
3. `docs/time_tracer/architecture/library_dependency_map.md`
4. `docs/time_tracer/core/specs/AGENT_ONBOARDING.md`

## Read By Task

- Capability ownership or dependency direction:
  - `docs/time_tracer/core/architecture/tracer_core_capability_dependency_map.md`
  - `docs/time_tracer/core/design/tracer_core_capability_boundary_contract.md`
- TXT parsing, authored events, mixed timelines, or ingest order:
  - `docs/time_tracer/core/ingest/README.md`
  - `docs/time_tracer/core/ingest/interval_event_and_mixed_timeline_semantics.md`
  - `docs/time_tracer/core/design/ingest-persistence-boundary.md`
- TXT runtime DTO or action contract:
  `docs/time_tracer/core/contracts/text/runtime_txt_day_block_json_contract_v1.md`
- Tracer exchange or crypto:
  - `docs/time_tracer/core/contracts/crypto/tracer_exchange_package_v4.md`
  - `docs/time_tracer/core/contracts/crypto/runtime_crypto_json_contract_v1.md`
- Reporting or statistics semantics:
  `docs/time_tracer/core/contracts/stats/README.md`
- Validation behavior or error codes:
  - `docs/time_tracer/core/errors/error-codes.md`
  - `docs/time_tracer/core/architecture/README.md`

## Ownership Boundaries

- `pipeline` owns month-TXT authoring, parsing, validation, normalization, and
  day-block semantics.
- `tracer_core` owns capability DTO meaning and application behavior.
- `tracer_transport` owns JSON envelope and codec mechanics.
- `tracer_core_bridge_common` owns reusable bridge helpers only.
- `tracer_adapters_io` owns filesystem-facing IO, not business validation.
- Host/runtime wiring stays outside the aggregate `TracerCoreRuntime`; shared
  runtime bridge helpers belong under `src/application/runtime_bridge/**`.

## Tests And Assets

- Keep core semantic tests under `libs/tracer_core/tests/**`.
- Use `test/fixtures/text/**` and `test/fixtures/config/**` for small shared
  file-based cases.
- Treat `test/data/**` as canonical cross-client TXT input.
- Use `test/golden/**` only for stable final-output reconciliation.
- Do not introduce checked-in database intermediate state.

## Validation

Required for core code, config, or test changes:

```powershell
python tools/run.py verify --app tracer_core_shell --profile fast --concise
```

For a single clearly owned capability, the corresponding `cap_*` profile may
replace `fast` when it covers every affected producer and consumer.

## Local Completion Bar

- The owning capability, DTO, and dependency direction remain explicit.
- Changed semantics have focused core regression coverage and synchronized
  owning contract documents.
- Core code, config, or test changes pass focused validation.
- A shell/runtime boundary change also passes `tracer_core_shell` verification.
- Shell/presentation code consumes core semantics instead of reimplementing
  them.
- If ingest changed, regression coverage proves persistence is not reached
  before all required validation succeeds.
