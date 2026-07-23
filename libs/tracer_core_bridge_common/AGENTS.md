# tracer_core_bridge_common Local Contract

## Scope

Applies to `libs/tracer_core_bridge_common/**`. This library contains reusable
shell-facing bridge helpers, not business capabilities.

## Read By Task

- Any bridge-helper change:
  `docs/time_tracer/architecture/libraries/tracer_core_bridge_common.md`
- Dependency or ownership change:
  `docs/time_tracer/architecture/library_dependency_map.md`
- C ABI bridge mapping or payload change:
  `docs/time_tracer/core/shared/c_abi.md`
- TXT behavior change: read `libs/tracer_core/AGENTS.md` and move the semantic
  change upstream.

## Ownership Boundaries

- Own only helpers reused by shell/runtime bridge implementations.
- Do not own TXT authoring, day-block parsing, validation, replacement, or
  runtime action semantics.
- Do not create a separate bridge-specific business model or test-asset layer.
- Runtime output and test-execution state belong under `out/test/**`, not
  `test/**`; agent scratch files still follow the repository `temp/` rule.

## Tests And Assets

- Prefer existing `test/fixtures/config/**` or downstream app fixtures when a
  bridge test genuinely requires a file.
- Treat `test/data/**` as cross-client canonical input, not bridge-owned data.

## Validation

Required for bridge-helper code, config, or test changes:

```powershell
python tools/run.py verify --app tracer_core_shell --profile fast --concise
```

The same verification covers shell-facing integration and the C ABI boundary;
update the canonical ABI document when that boundary changes.

## Local Completion Bar

- Changed helper behavior has focused bridge or shell-boundary coverage.
- Bridge-helper code, config, or test changes pass focused validation.
- A shell/C ABI change also passes `tracer_core_shell` verification and updates
  the canonical ABI document.
- No core business rule is implemented locally.
