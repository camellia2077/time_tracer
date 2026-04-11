# tracer_transport

## Purpose

Detailed navigation for the transport implementation library.

## When To Open

- Open this when the task changes runtime envelopes, field readers, runtime DTO helpers, or transport codec behavior.
- Open this after `library_dependency_map.md` once `tracer_transport` is the confirmed owner.

## What This Doc Does Not Cover

- ABI semantic ownership
- Business-rule changes
- Host runtime lifecycle behavior

## Ownership

1. `tracer_transport` owns transport implementation, not contract meaning.
2. It centralizes envelopes, field readers, runtime DTO helpers, and codec logic.
3. External contract meaning still lives in docs and host-facing boundaries.

## TXT Runtime Family Index

1. `tracer_transport` owns TXT runtime envelope/codec mechanics such as
   `runtime_txt_json` capability projection and JSON envelope normalization.
2. It does not own TXT day-block business semantics, `MMDD` validation rules,
   or action meaning for `default_day_marker`, `resolve_day_block`, and
   `replace_day_block`.
3. For TXT semantic ownership, route upstream to `tracer_core` and the TXT
   runtime contract doc.

## Allowed Dependencies

1. Standard library
2. `nlohmann_json`

## Forbidden Dependencies

1. Do not place `tracer_core` business rules here.
2. Do not place JNI/C API host lifecycle glue here.
3. Do not make this library the sole source of truth for ABI semantics.

## Public Surfaces

1. `include/tracer/transport/envelope.hpp`
2. `include/tracer/transport/fields.hpp`
3. `include/tracer/transport/runtime_requests.hpp`
4. `include/tracer/transport/runtime_responses.hpp`
5. `include/tracer/transport/runtime_codec.hpp`
6. Canonical modules under `src/modules`

## Change Routing

1. Change envelope wrapper behavior:
   - start in `include/tracer/transport/envelope.hpp` and `src/envelope.cpp`
2. Change field parsing or normalization:
   - start in `include/tracer/transport/fields.hpp` and `src/fields.cpp`
3. Change runtime request/response DTO helpers:
   - start in `include/tracer/transport/runtime_requests.hpp` and `include/tracer/transport/runtime_responses.hpp`
4. Change operation codec behavior:
   - start in the matching `src/runtime_codec_*.cpp` family
   - treat these as operation codec entrypoints, not as a complete file checklist
5. Change module surfaces:
   - inspect `src/modules` and module smoke tests
6. Change TXT runtime capability projection or envelope behavior:
   - start in `src/runtime_codec_capabilities.cpp`
   - then inspect shared runtime request/response helpers
   - if field meaning changes, leave this library and update the TXT runtime
     contract plus `tracer_core` owner docs instead

## Tests / Validate Entry Points

1. Envelope, field, runtime codec, and module smoke tests live under `tests/`.
2. TXT runtime capability coverage currently includes:
   - `libs/tracer_transport/tests/transport_runtime_codec_encode_tests.cpp`
   - downstream shell/runtime integration coverage in
     `apps/tracer_core_shell/tests/integration/**`
3. Focused validation:

```powershell
python tools/run.py validate --plan <plan_name> --paths libs/tracer_transport apps/tracer_core_shell
```

## Test Intent

1. `libs/tracer_transport/tests/**` 主要保护 envelope / field / codec contract，
   不负责定义 `tracer_core` 业务语义。
2. runtime decode 测试应重点覆盖：
   - 字段类型错误时的拒绝行为
   - 可选字段缺失时的默认回落
   - 错误 contract 字段向响应 payload 的稳定投影
3. `tree` / runtime response 的缺字段默认行为回归用于保护以下约定：
   - 缺失 `error_code` 时默认回落为空字符串
   - 缺失 `error_category` 时默认回落为空字符串
   - 缺失 `hints` 时默认回落为空数组
   - 这类缺失不会自动升级成解析失败，只要必需字段仍满足 contract
4. 如果 transport 测试开始表达“某个业务场景算不算错误”，应回查 owner
   是否其实在 `tracer_core` 或 contract 文档，而不是继续把语义塞进 transport
   测试说明。

## Read-First Docs

1. [Library Dependency Map](../library_dependency_map.md)
2. [C ABI Contract](../../core/contracts/c_abi.md)
3. [Android Runtime Protocol](../../presentation/android/runtime-protocol.md)
4. [TXT Runtime JSON Contract](../../core/contracts/text/runtime_txt_day_block_json_contract_v1.md)
5. [tracer_core](tracer_core.md)
