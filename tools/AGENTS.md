# Toolchain Local Contract

## Scope

Applies to `tools/**`. This subtree owns repository build/verify orchestration,
suite execution infrastructure, suite definitions, schema linting, and
toolchain self-tests.

## Required Read Set

1. `docs/tools/toolchain/README.md`
2. `docs/tools/toolchain/tools/README.md`
3. `docs/tools/toolchain/test/README.md`

## Read By Task

- Command or handler routing:
  `docs/tools/toolchain/command_map/README.md`
- Suite ownership, output contract, or test layering:
  - `docs/tools/toolchain/test/test_layering.md`
  - `tools/suites/README.md`
- Workflow orchestration: `docs/tools/toolchain/workflows/README.md`
- Clang-tidy flows: `docs/tools/toolchain/tidy/README.md`
- Developer helper scripts: `tools/scripts/AGENTS.md`

## Ownership And Routing

- `tools/run.py`: build, verify, analyze, tidy, and self-test entry.
- `tools/test.py`: product suite and runtime-guard entry.
- `tools/lint_suites.py`: suite-schema lint entry.
- `tools/toolchain/**`: command implementation and orchestration.
- `tools/test_framework/**`: suite runner, loader, guard, and result contract.
- `tools/suites/**`: product suite definitions and suite-local scripts.
- `tools/tests/**`: toolchain unit/component tests.
- `test/**`: shared inputs, fixtures, and golden assets only; it does not own
  suite or runner implementation.

## Local Invariants

- Do not move suite definitions or test-framework implementation back under
  `test/**`.
- Command/protocol changes update the owning docs under
  `docs/tools/toolchain/**`.
- Result-contract or suite-loader changes also update `tools/suites/README.md`
  and the test-layering document when ownership changes.
- Keep `tools/README.md` and this file as thin routers; detailed workflow and
  implementation knowledge belongs under `docs/tools/toolchain/**`.

## Validation

For toolchain command, handler, orchestration, or result-contract changes:

```powershell
python tools/run.py self-test
```

For suite TOML, suite discovery, or schema changes:

```powershell
python tools/lint_suites.py
```

Also run the affected product's local verification when toolchain behavior
changes what that product builds, executes, or reports.

## Local Completion Bar

- Changed command/handler behavior has focused coverage under `tools/tests/**`.
- Changed suite schemas pass suite lint; changed suite behavior is exercised by
  the owning product suite.
- Output/result-contract changes have producer and consumer coverage and updated
  contract documentation.
- The required self-test, suite lint, and affected product verification pass for
  every applicable change category.
