# Core Crypto Contracts

本目录定义 `tracer_core` 的加密文件契约（`.tracer`），用于 Windows CLI / Android / 其他端统一互通。

## Migration Note
1. Capability-first exchange routing now starts at
   `docs/time_tracer/core/capabilities/exchange/README.md`.
2. This directory still holds the detailed exchange/crypto contract set.

## Group Indexes
1. `docs/time_tracer/core/contracts/crypto/file_format/README.md`
   - 外层 `.tracer` 文件格式分组。
2. `docs/time_tracer/core/contracts/crypto/package/README.md`
   - tracer-exchange package 契约分组。
3. `docs/time_tracer/core/contracts/crypto/runtime/README.md`
   - runtime JSON / progress / error model 分组。

## Flat Docs Retained For Compatibility
1. `docs/time_tracer/core/contracts/crypto/file_format_v1.md`
2. `docs/time_tracer/core/contracts/crypto/file_format_v2.md`
3. `docs/time_tracer/core/contracts/crypto/tracer_exchange_package_v4.md`
4. `docs/time_tracer/core/contracts/crypto/tracer_exchange_package_v2.md`
5. `docs/time_tracer/core/contracts/crypto/tracer_exchange_package_v3.md`
6. `docs/time_tracer/core/contracts/crypto/error_model_v1.md`
7. `docs/time_tracer/core/contracts/crypto/progress_callback_v1.md`
8. `docs/time_tracer/core/contracts/crypto/runtime_crypto_json_contract_v1.md`

## 约束
1. `.tracer` 格式必须带 `magic + version`，禁止仅靠扩展名识别。
2. 任何破坏性变更必须升级 `version`，禁止 silent break。
3. 密钥与口令不写入日志、不写入错误文件。
4. 进度回调字段必须保持跨宿主一致（Android / Windows C ABI 同源映射）。
5. 当前 Windows tracer exchange 流程下，`file_format_v2` 的明文 payload 固定为 `tracer_exchange_package_v4`。
