# Core Crypto Contracts

本目录定义 `tracer_core` 的加密交换文件契约（当前为密码保护的 `.zip`），
用于 Windows CLI / Android / 其他端统一互通。

## Migration Note
1. Capability-first exchange routing now starts at
   `docs/time_tracer/core/capabilities/exchange/README.md`.
2. This directory still holds the detailed exchange/crypto contract set.

## Group Indexes
1. `docs/time_tracer/core/contracts/crypto/file_format/README.md`
   - 外层 ZIP AES-256 文件格式分组。
2. `docs/time_tracer/core/contracts/crypto/package/README.md`
   - tracer-exchange package 契约分组。
3. `docs/time_tracer/core/contracts/crypto/runtime/README.md`
   - runtime JSON / progress / error model 分组。

## Flat Docs
1. `docs/time_tracer/core/contracts/crypto/tracer_exchange_package_v6.md`
2. `docs/time_tracer/core/contracts/crypto/error_model_v1.md`
3. `docs/time_tracer/core/contracts/crypto/runtime_crypto_json_contract_v1.md`

## 约束
1. 交换导出必须是真正的 ZIP 文件，并使用 ZIP AES-256 entry 加密；禁止
   仅改扩展名伪装为 ZIP。
2. ZIP AES extra field、PBKDF2 参数和 entry 布局属于稳定契约，变更时必须
   更新对应版本文档。
3. 密钥与口令不写入日志、不写入错误文件。
4. 进度回调字段必须保持跨宿主一致（Android / Windows C ABI 同源映射）。
   加密 ZIP 导出只报告 package-level 总体进度；当前文件进度字段不用于
   该导出流程，导入等其他 exchange 流程仍可使用当前文件字段。
5. 当前 Windows tracer exchange 流程下，ZIP entries 的逻辑内容固定由
   `tracer_exchange_package_v6` 定义。
