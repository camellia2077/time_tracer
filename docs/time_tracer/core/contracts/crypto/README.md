# Core Crypto Contracts

本目录定义 `tracer_core` 的加密文件契约（`.tracer`），用于 Windows CLI / Android / 其他端统一互通。

## 文档列表
1. `docs/time_tracer/core/contracts/crypto/file_format_v1.md`
   - `.tracer` 二进制格式 `v1` 字段定义与解析约束。
2. `docs/time_tracer/core/contracts/crypto/file_format_v2.md`
   - `.tracer` 二进制格式 `v2`（zstd 后加密）字段定义与解析约束。
3. `docs/time_tracer/core/contracts/crypto/error_model_v1.md`
   - 加密/解密错误模型、稳定错误码与对外行为。
4. `docs/time_tracer/core/contracts/crypto/progress_callback_v1.md`
   - 进度回调字段、阶段语义、节流与取消约束。
5. `docs/time_tracer/core/contracts/crypto/runtime_crypto_json_contract_v1.md`
   - C ABI `runtime_crypto_*_json` 的请求字段、默认值、路径语义与响应 envelope。
   - 进度事件通过独立 callback side channel 提供，不混入 response envelope。

## 约束
1. `.tracer` 格式必须带 `magic + version`，禁止仅靠扩展名识别。
2. 任何破坏性变更必须升级 `version`，禁止 silent break。
3. 密钥与口令不写入日志、不写入错误文件。
4. 进度回调字段必须保持跨宿主一致（Android / Windows C ABI 同源映射）。
