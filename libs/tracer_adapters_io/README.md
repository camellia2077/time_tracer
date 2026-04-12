# tracer_adapters_io

## Purpose

Local entrypoint for agents touching the IO adapter library.

## When To Open

- Open this when the task touches `libs/tracer_adapters_io`.
- Use it to jump into the detailed adapter doc and the cross-library map.

## What This Doc Does Not Cover

- Internal adapter-family routing
- Detailed module ownership notes
- Full validation inventory

## Open Next

1. [Detailed library doc](../../docs/time_tracer/architecture/libraries/tracer_adapters_io.md)
2. [Library dependency map](../../docs/time_tracer/architecture/library_dependency_map.md)
3. [Detailed `tracer_core` doc](../../docs/time_tracer/architecture/libraries/tracer_core.md)

## Validation Boundary

- `tracer_adapters_io` does not own TXT/TOML business validation.
- For converter-config validation and TXT structure/logic validation, open [libs/tracer_core/README.md](../tracer_core/README.md) and follow its `Validation Docs` section.

## Capability / Ownership Index

1. `tracer_adapters_io` owns filesystem-facing TXT read/write helpers only.
2. It does not own month-TXT day-block extraction, replacement, default
   `MMDD` selection, or TXT runtime DTO semantics.
3. Shared TXT day-block rules live in `tracer_core` pipeline-owned semantics
   and are exposed to hosts through the TXT runtime family.

## Test Asset Boundary

1. `tracer_adapters_io` 的测试源码继续放在 `libs/tracer_adapters_io/tests/**`。
2. 因为它会消费实际文件，`test/fixtures/text/**` 对它是合理的小型样本来源。
3. `test/data/**` 仍是跨端 canonical TXT 输入，不是 adapters_io 自己的
   私有 fixture 目录。
4. day-block 语义、默认 `MMDD` 与业务规则仍归 `tracer_core`，不要因为文件
   读写发生在这里就扩大 owner。
5. 运行结果目录仍应落在 `out/test/**`，不要新增 `test/output/**` 依赖。

## Validate

```powershell
python tools/run.py validate --plan <plan_name> --paths libs/tracer_adapters_io apps/tracer_core_shell
```
