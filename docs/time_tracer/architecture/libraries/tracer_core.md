# tracer_core

## Purpose

Detailed navigation for the core business-logic library.

## When To Open

- Open this when the task changes core business rules, use cases, workflow, or core-owned infrastructure behavior.
- Open this after `library_dependency_map.md` once `tracer_core` is the confirmed owner.

## What This Doc Does Not Cover

- Full file inventories
- Historical refactor notes
- Exact implementation steps for every change

## Ownership

1. `tracer_core` is the business-logic owner.
2. It owns domain/application boundaries, capability-owned DTO families, and internal infrastructure families.
3. It does not own runtime JSON wire semantics or host bridge lifecycle glue.

## Allowed Dependencies

1. `shared`
2. `domain -> shared`
3. `application -> domain + shared`
4. `infrastructure -> shared + domain + application`

## Forbidden Dependencies

1. `domain` and `application` must not depend on:
   - `libs/tracer_transport`
   - `libs/tracer_adapters_io`
   - `libs/tracer_core_bridge_common`
   - `apps/**`
2. `domain` and `application` must not expose `nlohmann::json`.
3. Runtime envelope ownership does not live here.

## Public Surfaces

1. Canonical module surfaces under `src/*/modules`
2. Explicit boundary declaration headers under `src/**`
3. `ITracerCoreRuntime` as the only aggregate runtime surface for shell-facing consumers
   and as a composition bridge over prebuilt capability APIs rather than a
   capability-specific wiring entrypoint
4. Capability APIs, capability-owned `application/ports/<capability>/**` subtrees,
   and capability-owned DTO headers under `src/application/dto/{pipeline,query,reporting,exchange}_*.hpp`
   consumed by shell-facing code and tests
5. Explicit non-owner families under `src/application/dto/compat/**`,
   `src/application/compat/reporting/**`, `src/application/aggregate_runtime/**`,
   and `src/application/runtime_bridge/**`
6. Retained workflow declaration boundary under `src/application/interfaces/i_workflow_handler.hpp`
7. Legacy forwarding shim paths under `src/application/dto/core_*`,
   `src/application/interfaces/i_report_*`, and
   `src/application/use_cases/tracer_core_runtime*` are retired
8. Pipeline-owned TXT day-block semantics under
   `src/application/pipeline/txt_day_block_support.*`
9. Pipeline request/response DTO surfaces under
   `src/application/dto/pipeline_requests.hpp` and
   `src/application/dto/pipeline_responses.hpp`

## Reviewer Shortcut

1. Classify owner by capability-owned port subtree first:
   `src/application/ports/pipeline|query|reporting|exchange/**`
2. If the touched path is a capability-owned DTO header, route by DTO family:
   `src/application/dto/pipeline_*`, `query_*`, `reporting_*`, `exchange_*`
3. If the touched path is `src/application/compat/reporting/**`, treat it as
   retained `reporting` compatibility surface, not as canonical owner path
4. If the touched path is `src/application/interfaces/i_workflow_handler.hpp`,
   treat it as retained `pipeline` declaration boundary
5. Treat canonical module locations as the authority surface for reviewers:
   `src/application/modules/tracer.core.application.*` for application-owned capability modules and
   `src/infra/modules/<capability>/**` for infrastructure-owned capability modules
6. If the touched path is a non-owner family, route by family before capability:
   `src/application/dto/compat/**`, `src/application/aggregate_runtime/**`,
   `src/application/dto/shared_envelopes.hpp`, and `src/application/runtime_bridge/**`
7. Treat `src/application/runtime_bridge/**` as shell/runtime bridge surface,
   not as a capability-owned `application/ports/<capability>` subtree

## Change Routing

1. Change core use cases, workflow, or pipeline behavior:
   - read `docs/time_tracer/core/architecture/tracer_core_capability_dependency_map.md`
     and `docs/time_tracer/core/design/tracer_core_capability_boundary_contract.md`
   - start in `src/application/use_cases`, `src/application/workflow`, or `src/application/pipeline`
   - if the change is shell-visible, also inspect `apps/tracer_core_shell/api/c_api`
2. Change reporting/query semantics:
   - read `docs/time_tracer/core/architecture/tracer_core_capability_dependency_map.md`
     first to confirm whether the work belongs to `query` or `reporting`
   - start in `src/application/query/tree`, `src/application/reporting`, `src/infra/query`,
     or `src/infra/reporting`
   - read stats contract docs first if external meaning changes
3. Change config ownership or shell config bridges:
   - start in `src/infra/config`
   - then inspect `apps/tracer_core_shell/api/c_api` and `apps/tracer_core_shell/host`
4. Change persistence or schema behavior:
   - start in `src/infra/persistence` and `src/infra/schema`
5. Change module ownership or retained declaration boundaries:
   - inspect `src/*/modules`, explicit boundary headers, and `cmake/sources/*.cmake`
6. Change shell/runtime bridge, aggregate runtime, or compatibility envelope:
   - start in `src/application/runtime_bridge`, `src/application/aggregate_runtime`,
     `src/application/dto/shared_envelopes.hpp`, `src/application/dto/compat`, or
     `src/application/compat/reporting`
   - then inspect `apps/tracer_core_shell/host` and `apps/tracer_core_shell/api/c_api`
7. Change shared month-TXT day-block semantics or default `MMDD` selection:
   - start in `src/application/pipeline/txt_day_block_support.*`
   - then inspect `src/application/pipeline/pipeline_workflow.cpp`
   - if the change is host-visible, inspect
     `apps/tracer_core_shell/api/c_api/capabilities/txt/`,
     Android runtime bridging, and the Windows CLI `txt` handler

## Tests / Validate Entry Points

1. Layer smoke coverage lives under `tests/`.
2. Long-lived shell/runtime regressions live under `apps/tracer_core_shell/tests/platform`.
3. TXT day-block semantic coverage is grouped under:
   - `libs/tracer_core/tests/application/tests/modules/txt_day_block_tests.cpp`
   - `apps/tracer_core_shell/tests/integration/tracer_core_c_api_pipeline_tests.cpp`
   - `tools/suites/tracer_windows_rust_cli/tests/commands_txt_view_day.toml`
   - stage/log group `txt-view-day`
4. Focused validation:

```powershell
python tools/run.py validate --plan tools/toolchain/config/validate/tracer_core/query.toml
python tools/run.py verify --app tracer_core_shell --profile cap_query --concise
```

## Test Intent

1. `libs/tracer_core/tests/**` 主要保护业务语义，而不是 transport 级 JSON
   细节。
2. reporting / query 语义测试应优先说明“成功但为空”和“真正错误”之间的边界。
3. reporting 里的
   `empty success vs target not found`
   回归用于保护以下含义：
   - 空时间窗口的 report/range 结果仍然是成功结果
   - `has_records=false`、`matched_* = 0` 属于有效业务结果
   - 只有命名 target 不存在时，才返回稳定错误
     `reporting.target.not_found`
4. 当测试覆盖 capability-owned DTO、structured output 或 error contract
   传播时，文档应强调这是 `tracer_core` 语义边界，而不是 `tracer_transport`
   的 envelope 归一化职责。

## Read-First Docs

1. [Library Dependency Map](../library_dependency_map.md)
2. [Core Docs](../../core/README.md)
3. [Core Agent Onboarding](../../core/specs/AGENT_ONBOARDING.md)
4. [tracer_core Capability Dependency Map](../../core/architecture/tracer_core_capability_dependency_map.md)
5. [tracer_core Capability Boundary Contract](../../core/design/tracer_core_capability_boundary_contract.md)
6. [C ABI Contract](../../core/contracts/c_abi.md)
7. [Stats Contracts](../../core/contracts/stats/README.md)
8. [TXT Runtime JSON Contract](../../core/contracts/text/runtime_txt_day_block_json_contract_v1.md)

## TXT Day-Block Cross-Layer Call Chains

### Android DAY mode

1. `feature-record` Compose UI keeps presentation state such as mode, raw
   marker input, and editor visibility.
2. Android runtime client/service converts that state into TXT runtime action
   requests.
3. JNI forwards the request to `tracer_core_runtime_txt_json`.
4. Shell C ABI routes the action into pipeline-owned TXT DTO/workflow helpers.
5. `tracer_core` resolves or replaces the target day block and returns JSON for
   Android rendering and save gating.

### Windows CLI `txt view-day`

1. CLI parses arguments and reads the target TXT file locally.
2. CLI host code infers `selected_month` from the filename when possible.
3. CLI sends the full month content to `tracer_core_runtime_txt_json`.
4. Core pipeline TXT semantics resolve the requested block.
5. CLI prints `day_body` or reports a host-formatted error without re-encoding
   the month-TXT business rules locally.
