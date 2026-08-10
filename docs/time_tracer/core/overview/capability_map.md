# tracer_core Capability Map

## Purpose

This document is the overview-level authority map for `tracer_core`
capabilities. Use it when the change is already confirmed to belong to
`libs/tracer_core` and you need to know:
1. which capability owns the work
2. which direct capability dependencies are allowed
3. which focused verify profile should run first

## First-Class Capabilities

| Capability | Owns | Direct Capability Deps | Must Not Depend On |
| --- | --- | --- | --- |
| `pipeline` | convert / ingest / import / validate orchestration and workflow entry | `config`, `persistence_write` | `query`, `insights`, `exchange` |
| `query` | tree query semantics, data-query repository / orchestrators / renderers / stats | `config`, `persistence_runtime` | `insights` |
| `insights` | insights query / export / formatter / insights-data-query flows | `config`, `persistence_runtime` | `query` |
| `exchange` | tracer-exchange package flows and file-crypto-backed exchange implementation | `config` | `pipeline`, `query`, `insights` |
| `config` | runtime config loading, snapshotting, validators, insights/converter config assembly | none | capability orchestration |
| `persistence_write` | ingest/import write-side repositories and sqlite writer chain | support only | `query`, `insights`, `exchange` |
| `persistence_runtime` | read-side project repository, db health, shared sqlite support | none | capability orchestration |

Composition surfaces such as `application/use_cases/tracer_core_api.*`,
`application/aggregate_runtime/**`, `tc_core_iface`, and `tc_infra_full_lib`
may aggregate multiple capabilities, but they are not capability owners.

## Current Capability Graph

The target graph remains:
1. `pipeline -> config + persistence_write`
2. `query -> config + persistence_runtime`
3. `insights -> config + persistence_runtime`
4. `exchange -> config`

`persistence_write` may reuse `persistence_runtime` sqlite support internally,
but that does not become a new upward business-capability dependency.

```mermaid
flowchart LR
    Pipeline["pipeline"] --> Config["config"]
    Pipeline --> PWrite["persistence_write"]
    Query["query"] --> Config
    Query --> PRuntime["persistence_runtime"]
    Insights["insights"] --> Config
    Insights --> PRuntime
    Exchange["exchange"] --> Config
    PWrite -. "shared sqlite support" .-> PRuntime
```

## Verify First

1. `pipeline`
   - `python tools/run.py verify --app tracer_core_shell --profile cap_pipeline --concise`
2. `query`
   - `python tools/run.py verify --app tracer_core_shell --profile cap_query --concise`
3. `insights`
   - `python tools/run.py verify --app tracer_core_shell --profile cap_insights --concise`
4. `exchange`
   - `python tools/run.py verify --app tracer_core_shell --profile cap_exchange --concise`
5. `config`
   - `python tools/run.py verify --app tracer_core_shell --profile cap_config --concise`
6. `persistence_write`
   - `python tools/run.py verify --app tracer_core_shell --profile cap_persistence_write --concise`
7. `persistence_runtime`
   - `python tools/run.py verify --app tracer_core_shell --profile cap_persistence_runtime --concise`

## Read Next
1. [module_boundaries.md](module_boundaries.md)
2. [identity_and_boundary.md](identity_and_boundary.md)
3. [../capabilities/validation/README.md](../capabilities/validation/README.md)
4. [../capabilities/ingest/README.md](../capabilities/ingest/README.md)
