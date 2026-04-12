# tracer_core_bridge_common

## Purpose

Local entrypoint for agents touching the shared bridge-helper layer.

## When To Open

- Open this when the task touches `libs/tracer_core_bridge_common`.
- Use it to jump into the detailed bridge-helper doc and shell-facing contracts.

## What This Doc Does Not Cover

- Detailed bridge ownership notes
- Downstream shell/runtime routing
- Full validation and regression paths

## Open Next

1. [Detailed library doc](../../docs/time_tracer/architecture/libraries/tracer_core_bridge_common.md)
2. [Library dependency map](../../docs/time_tracer/architecture/library_dependency_map.md)
3. [C ABI contract](../../docs/time_tracer/core/contracts/c_abi.md)

## Capability / Ownership Index

1. `tracer_core_bridge_common` owns shared bridge helpers only.
2. It does not own TXT month authoring semantics, day-block parsing rules, or
   the `tracer_core_runtime_txt_json` action contract.
3. If the task changes TXT day-block behavior, move upstream to
   [libs/tracer_core/README.md](../tracer_core/README.md).

## Test Asset Boundary

1. `tracer_core_bridge_common` 不新增自己独立的测试资产层。
2. 若 bridge helper 测试需要小型文件资产，优先复用 `test/fixtures/config/**`
   或下游 app 的专用测试资产。
3. `test/data/**` 仍是跨端 canonical TXT 输入，不因为 bridge helper
   读取它而改变 owner。
4. 运行时输出与临时目录应进入 `out/test/**`，不要把新输出落回 `test/**`。

## Validate

```powershell
python tools/run.py validate --plan <plan_name> --paths libs/tracer_core_bridge_common apps/tracer_core_shell libs/tracer_transport
python tools/run.py verify --app tracer_core_shell --profile fast --scope batch --concise
```
