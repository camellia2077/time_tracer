# apps/time_tracer

`apps/time_tracer` 是 Core 实现与适配装配入口（源码在 `src/`）。

本 README 只提供文档索引，不复制规范正文。  
规范与契约统一放在根目录 `docs/`，避免多份文档漂移。

## 文档索引（权威）
1. 总入口：`docs/time_tracer/README.md`
2. Core 文档域：`docs/time_tracer/core/README.md`
3. C ABI 契约：`docs/time_tracer/core/contracts/c_abi.md`
4. 错误模型与日志契约：`docs/time_tracer/core/contracts/error-model.md`
5. 错误码定义：`docs/time_tracer/core/contracts/error-codes.md`
6. Ingest 数据结构：`docs/time_tracer/core/ingest/ingest_data_structures.md`
7. Ingest 转换算法：`docs/time_tracer/core/ingest/ingest_conversion_algorithms.md`
8. Windows CLI 文档：`docs/time_tracer/windows_cli/README.md`
9. Android UI 文档：`docs/time_tracer/android_ui/README.md`

## 约定
1. 改 Core 契约/规范，优先更新 `docs/time_tracer/core/*`。
2. `apps/*/README.md` 仅做入口索引，不承载跨端规范正文。
3. 平台特有说明放各自文档域，不回写到 Core 规范。

