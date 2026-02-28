# Crypto Error Model v1

## 目标
1. 给 CLI / Android 提供稳定、可判断的错误类别。
2. 保证失败时不泄露口令、密钥、明文内容。

## 错误类别（稳定枚举）
1. `kOk`
2. `kInvalidArgument`
3. `kInputReadFailed`
4. `kOutputWriteFailed`
5. `kUnsupportedFormat`
6. `kDecryptFailed`
7. `kCompressionFailed`
8. `kDecompressionFailed`
9. `kCompressionMetadataMismatch`
10. `kCryptoBackendUnavailable`
11. `kCryptoInitializationFailed`
12. `kCryptoOperationFailed`
13. `kCancelled`

## 稳定错误码映射
1. `kInvalidArgument` -> `crypto.invalid_argument`
2. `kInputReadFailed` -> `crypto.input_read_failed`
3. `kOutputWriteFailed` -> `crypto.output_write_failed`
4. `kUnsupportedFormat` -> `crypto.unsupported_format`
5. `kDecryptFailed` -> `crypto.decrypt_failed`
6. `kCompressionFailed` -> `crypto.compress_failed`
7. `kDecompressionFailed` -> `crypto.decompress_failed`
8. `kCompressionMetadataMismatch` -> `crypto.compression_metadata_mismatch`
9. `kCryptoBackendUnavailable` -> `crypto.backend_unavailable`
10. `kCryptoInitializationFailed` -> `crypto.init_failed`
11. `kCryptoOperationFailed` -> `crypto.operation_failed`
12. `kCancelled` -> `crypto.cancelled`

## 行为约束
1. 口令错误与密文篡改统一归类为 `kDecryptFailed`，不泄露更多判别信息。
2. 失败场景不得留下半写入明文文件（调用方需在失败时清理目标文件）。
3. 日志与错误信息不得包含：
   - 原始口令
   - 派生密钥
   - 明文内容
4. `kCompressionMetadataMismatch` 用于标记 header 元信息与解压结果不一致。
5. `kCancelled` 用于标记上层主动取消（callback 或 cancel token）。

## 调用方建议
1. CLI 对 `kDecryptFailed` 输出通用提示：口令错误或文件损坏。
2. Android UI 仅给用户可操作提示，不暴露底层算法细节。
