# Crypto Progress Callback v1

## 目标
1. 为 CLI / Android 提供统一进度事件，不把任何 UI 逻辑放进 core。
2. 支持双层进度：总体进度（跨目录全量）+ 当前文件进度。
3. 支持节流与取消，避免高频回调造成前端卡顿。

## API 入口
1. 单文件：
   - `EncryptFile(input, output, passphrase, options)`
   - `DecryptFile(input, output, passphrase, options)`
2. 批量目录：
   - `EncryptDirectory(input_root, output_root, passphrase, options)`
   - `DecryptDirectory(input_root, output_root, passphrase, options)`

## 进度快照字段（`FileCryptoProgressSnapshot`）
1. `operation`：`kEncrypt` / `kDecrypt`
2. `phase`：
   - `kScan`
   - `kReadInput`
   - `kCompress`
   - `kDeriveKey`
   - `kEncrypt`
   - `kDecrypt`
   - `kDecompress`
   - `kWriteOutput`
   - `kCompleted`
   - `kCancelled`
   - `kFailed`
3. 总体：
   - `overall_done_bytes`
   - `overall_total_bytes`
   - `total_files`
4. 当前文件：
   - `current_input_path`
   - `current_output_path`
   - `current_file_done_bytes`
   - `current_file_total_bytes`
   - `current_file_index`
5. 分组（用于年份目录等层级展示）：
   - `current_group_label`
   - `group_index`
   - `group_count`
   - `file_index_in_group`
   - `file_count_in_group`

## 分组规则
1. 批量模式按 `input_root` 下“相对路径第一层目录”分组。
2. 文件直接位于根目录时，分组为 `(root)`。
3. 该分组可直接映射到“年份进度 + 年份内文件进度”显示。

## 节流与取消
1. `FileCryptoOptions.progress_min_interval`：最小回调时间间隔（默认 100ms）。
2. `FileCryptoOptions.progress_min_bytes_delta`：最小字节增量（默认 64 KiB）。
3. `FileCryptoOptions.progress_callback` 返回 `kCancel` 时，流程中断。
4. `FileCryptoOptions.cancel_token` 可由外部主动触发取消。
5. 取消统一返回错误：`kCancelled` / `crypto.cancelled`。

## C ABI 侧边通道（v1 增量）
1. C ABI 注册函数：
   - `tracer_core_set_crypto_progress_callback(TtCoreCryptoProgressCallback callback, void* user_data)`
2. 回调签名：
   - `void callback(const char* utf8_progress_json, void* user_data)`
3. 清理语义：
   - 传 `nullptr` callback 等价于清空注册。
4. 稳定性语义：
   - host 回调异常不得中断 core 主流程。
   - `utf8_progress_json` 仅在回调调用期间有效，host 需自行复制。

## JSON 映射字段（C ABI / Android 同源）
1. `operation`
2. `phase`
3. `current_group_label`
4. `group_index`
5. `group_count`
6. `file_index_in_group`
7. `file_count_in_group`
8. `current_file_index`
9. `total_files`
10. `current_file_done_bytes`
11. `current_file_total_bytes`
12. `overall_done_bytes`
13. `overall_total_bytes`
14. `speed_bytes_per_sec`
15. `remaining_bytes`
16. `eta_seconds`
17. `current_input_path`
18. `current_output_path`
19. `input_root_path`
20. `output_root_path`

## 对齐约束
1. Android `onCryptoProgressJson` 与 C ABI `tracer_core_set_crypto_progress_callback` 共享同一字段映射逻辑。
2. 任一字段新增必须保持“可选/加法”兼容，不得破坏既有解析端。
