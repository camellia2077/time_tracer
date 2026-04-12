# tracer_transport

## Purpose

Local entrypoint for agents touching the shared transport implementation layer.

## When To Open

- Open this when the task touches `libs/tracer_transport`.
- Use it to jump into the detailed library doc and ABI/runtime contract docs.

## What This Doc Does Not Cover

- Codec-family routing details
- Transport boundary rules
- Full test inventory

## Open Next

1. [Detailed library doc](../../docs/time_tracer/architecture/libraries/tracer_transport.md)
2. [Library dependency map](../../docs/time_tracer/architecture/library_dependency_map.md)
3. [C ABI contract](../../docs/time_tracer/core/contracts/c_abi.md)

## Capability / Ownership Index

1. `tracer_transport` owns runtime JSON envelope encoding/decoding and
   capability feature-flag projection such as `runtime_txt_json`.
2. `tracer_transport` does not own the meaning of TXT runtime actions or the
   month-TXT day-block rules behind them.
3. For TXT action semantics, open [libs/tracer_core/README.md](../tracer_core/README.md)
   and the [TXT runtime JSON contract](../../docs/time_tracer/core/contracts/text/runtime_txt_day_block_json_contract_v1.md).

## Tests / Semantics Covered

1. `libs/tracer_transport/tests/**` 主要保护 envelope / codec contract 与默认字段回落行为。
2. `tree` / runtime response 回归会覆盖错误 contract 字段缺失时的默认 decode 行为。
3. 详细测试意图见
   [docs/time_tracer/architecture/libraries/tracer_transport.md](../../docs/time_tracer/architecture/libraries/tracer_transport.md).

## Test Asset Boundary

1. `tracer_transport` 的测试源码继续放在 `libs/tracer_transport/tests/**`。
2. transport 测试默认优先使用库内 inline payload，而不是把业务语义样本写进
   `test/**`。
3. 只有跨层共享、且确实需要文件化的小型 payload，才考虑放入
   `test/fixtures/**`。
4. `test/data/**` 是主程序消费的 canonical TXT 输入，不是 transport
   专用样本池。
5. `test/golden/**` 是最终输出对账基线，不属于 transport 自身的 codec
   回归资产。

## Validate

```powershell
python tools/run.py validate --plan <plan_name> --paths libs/tracer_transport apps/tracer_core_shell
```
