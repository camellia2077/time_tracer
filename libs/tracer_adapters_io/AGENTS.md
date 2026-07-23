# tracer_adapters_io Local Contract

## Scope

Applies to `libs/tracer_adapters_io/**`. This library owns filesystem-facing IO
adapters for shared tracer data.

## Read By Task

- Any adapter change:
  `docs/time_tracer/architecture/libraries/tracer_adapters_io.md`
- Dependency or ownership change:
  `docs/time_tracer/architecture/library_dependency_map.md`
- TXT validation or ingest semantics: read `libs/tracer_core/AGENTS.md` and the
  core documents it routes to.

## Ownership Boundaries

- Own reading and writing files plus adapter-level filesystem errors.
- Do not own TXT/TOML business validation, day-block semantics, default day
  selection, or runtime DTO meaning.
- Do not duplicate parsing or validation rules from `tracer_core`.

## Tests And Assets

- Keep adapter tests under `libs/tracer_adapters_io/tests/**`.
- `test/fixtures/text/**` is appropriate for small filesystem cases.
- Treat `test/data/**` as cross-client canonical input, not private fixtures.
- Write runtime results under `out/test/**`; do not create `test/output/**`.

## Validation

Required for adapter code, config, or test changes:

```powershell
python tools/run.py verify --app tracer_core_shell --profile fast --concise
```

## Local Completion Bar

- Changed read, write, and filesystem-error paths have focused coverage.
- Adapter code, config, or test changes pass the focused validation above.
- Error handling preserves the adapter/core boundary without duplicating core
  validation.
- No generated output is written back into test input directories.
