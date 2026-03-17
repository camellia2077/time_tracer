# tracer_core

`tracer_core` is the main engine library for `time_tracer`. It owns core
business rules, application orchestration, module-owned declarations, and the
internal infrastructure families that implement config, persistence, query,
reports, logging, crypto, and platform support.

## Start Here
1. [Detailed library doc](../../docs/time_tracer/architecture/libraries/tracer_core.md)
2. [Library dependency map](../../docs/time_tracer/architecture/library_dependency_map.md)

## Validate

```powershell
python tools/run.py validate --plan <plan_name> --paths libs/tracer_core apps/tracer_core_shell
python tools/run.py verify --app tracer_core_shell --quick --scope batch --concise
```

## Read-First Docs
1. [Core docs](../../docs/time_tracer/core/README.md)
2. [Core agent onboarding](../../docs/time_tracer/core/specs/AGENT_ONBOARDING.md)
3. [C ABI contract](../../docs/time_tracer/core/contracts/c_abi.md)
