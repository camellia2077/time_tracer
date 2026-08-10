# log_generator Local Contract

## Scope

Applies to `apps/tools/log_generator/**`. This tool app generates canonical TXT
datasets consumed by repository validation, ingest, query, insights, and golden
comparison flows.

## Required Read Set

1. `apps/tools/log_generator/README.md`
2. `docs/tools/log_generator/usage.md`
3. `test/README.md`
4. `docs/tools/toolchain/test/README.md`

## Read By Task

- Test layering or asset ownership:
  `docs/tools/toolchain/test/test_layering.md`
- TXT ingest semantics:
  - `docs/time_tracer/core/ingest/README.md`
  - `docs/time_tracer/core/ingest/txt_to_db_business_logic.md`
- Interval or mixed-timeline behavior:
  `docs/time_tracer/core/ingest/interval_event_and_mixed_timeline_semantics.md`

## Ownership And Routing

- CLI parsing: `src/main.cpp` and `src/cli/**`.
- Workflow/config wiring: `src/application/**` and `src/infrastructure/config/**`.
- Generation semantics: `src/domain/**`.
- Suite definitions: `tools/suites/log_generator/**`.
- The generator owns dataset generation policy and self-checks; it does not own
  the downstream core TXT contract.

## Local Invariants

- Keep the tool under `apps/tools/log_generator`; do not move it into `test/**`.
- Do not write ad-hoc output into `test/data/**` or `test/golden/**` unless the
  task explicitly refreshes canonical assets.
- A generated-TXT shape or timeline change requires review of generator
  self-checks, suite guards, downstream consumers, and golden expectations.
- Reuse `out/build/log_generator/build_fast` for incremental verification unless
  the task requires a different build directory.

## Validation

Required for generator code, config, suite, or behavior changes:

```powershell
python tools/run.py verify --app log_generator --build-dir build_fast --concise
```

Evidence:

- `out/test/artifact_log_generator/result.json`
- `out/test/artifact_log_generator/result_cases.json`
- `out/test/artifact_log_generator/logs/output.log`

## Local Completion Bar

- Generator behavior, self-checks, and suite guards agree on the TXT shape and
  the required verify flow passes.
- Canonical asset refreshes occur only when explicitly in scope and include a
  reviewable downstream-impact insights.
- A generation-policy change has deterministic seeded coverage for its new or
  changed branches.
