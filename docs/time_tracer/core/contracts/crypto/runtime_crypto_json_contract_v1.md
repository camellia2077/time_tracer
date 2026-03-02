# Core Runtime Crypto JSON Contract v1

## 状态
1. 状态：Active
2. 适用范围：`tracer_core_runtime_crypto_encrypt_json` / `decrypt_json` / `inspect_json`
3. 目标：定义 C ABI 下 crypto JSON 请求与响应的稳定语义，供 Windows CLI / Android / 其他 host 统一接入。

## 1. 对外 ABI 入口
1. `const char* tracer_core_runtime_crypto_encrypt_json(TtCoreRuntimeHandle*, const char* request_json)`
2. `const char* tracer_core_runtime_crypto_decrypt_json(TtCoreRuntimeHandle*, const char* request_json)`
3. `const char* tracer_core_runtime_crypto_inspect_json(TtCoreRuntimeHandle*, const char* request_json)`

说明：
1. `request_json` 必须是 UTF-8 JSON object 且不能为空。
2. 返回值是线程局部 UTF-8 JSON 字符串，下一次同线程 ABI 调用会覆盖它。

## 2. 通用请求规则
1. 请求必须为 JSON object；空字符串或非 object 直接失败。
2. 路径字段统一按 `std::filesystem::absolute` 解析为绝对路径再执行。
3. 扩展名判断大小写不敏感（例如 `.TXT` / `.txt` 均可匹配）。
4. 必填字符串字段必须非空；为空会报 `field \`...\` must be a non-empty string.`。
5. 未定义字段当前会被忽略（不报错），但不保证未来版本继续忽略；host 应仅发送契约字段。

## 3. Encrypt 接口契约

### 3.1 请求字段
1. `input_path`（必填，string）：输入文件或目录路径。
2. `output_path`（必填，string）：输出文件或目录路径。
3. `passphrase`（必填，string）：口令。
4. `date_check_mode`（可选，string）：`none|continuity|full`，默认 `none`。
5. `security_level`（可选，string）：`min|interactive|moderate|high|max`，默认 `interactive`。
   - 兼容别名：`sensitive` 等价于 `high`。

### 3.2 处理语义
1. 输入不存在时失败：`Input path does not exist: ...`
2. 输入为单文件时：
   - 必须是 `.txt`，否则失败：`Encrypt input file must be .txt: ...`
   - 会先执行结构校验和逻辑校验，再执行加密。
   - 若 `output_path` 已存在且是目录，则实际输出为 `<output_path>/<input_filename>.tracer`。
   - 否则将 `output_path` 扩展名替换为 `.tracer` 作为最终输出文件。
3. 输入为目录时：
   - `output_path` 不能是已存在的普通文件。
   - 会执行目录级校验，再批量加密目录内 `.txt` 文件。

### 3.3 成功 `content` 文本
1. 单文件：`Encrypted: <input_abs> -> <output_abs>`
2. 目录：`Encrypted <N> txt files to: <output_abs>`

## 4. Decrypt 接口契约

### 4.1 请求字段
1. `input_path`（必填，string）：输入文件或目录路径。
2. `output_path`（必填，string）：输出文件或目录路径。
3. `passphrase`（必填，string）：口令。

说明：
1. `decrypt` 不解析 `security_level`。
2. `decrypt` 不解析 `date_check_mode`。
3. 即使携带上述字段，当前实现也会忽略它们，不参与行为计算。

### 4.2 处理语义
1. 输入不存在时失败：`Input path does not exist: ...`
2. 输入为单文件时：
   - 必须是 `.tracer`，否则失败：`Decrypt input file must be .tracer: ...`
   - 输出路径规则与 encrypt 对称：目录则 `<dir>/<input_filename>.txt`，否则替换扩展名为 `.txt`。
3. 输入为目录时：
   - `output_path` 不能是已存在的普通文件。
   - 按目录批量解密 `.tracer` 文件。

### 4.3 成功 `content` 文本
1. 单文件：`Decrypted: <input_abs> -> <output_abs>`
2. 目录：`Decrypted <N> tracer files to: <output_abs>`

## 5. Inspect 接口契约

### 5.1 请求字段
1. `input_path`（必填，string）：目标 `.tracer` 文件或目录。

### 5.2 处理语义
1. 输入不存在时失败：`Input path does not exist: ...`
2. 输入为单文件时：直接读取该文件头部元信息并输出文本。
3. 输入为目录时：
   - 递归收集目录下所有 `.tracer` 文件（按路径排序）。
   - 若未找到任何 `.tracer`，失败：`No .tracer files found under directory: ...`
   - 多文件结果之间用空行分隔。

### 5.3 成功 `content` 文本字段
1. `File: <path>`
2. `version`
3. `kdf_id`
4. `cipher_id`
5. `compression_id`
6. `compression_level`
7. `ops_limit`
8. `mem_limit_kib`
9. `plaintext_size`
10. `ciphertext_size`

## 6. 通用响应契约
返回统一 JSON envelope（UTF-8）：
1. `ok`（bool）
2. `content`（string）
3. `error_message`（string）
4. `error_code`（string）
5. `error_category`（string）
6. `hints`（string[]）

说明：
1. 成功时：`ok=true`，`error_message=""`，`error_code=""`，`error_category=""`，`hints=[]`。
2. 失败时：当前这 3 个 crypto C ABI 接口统一走 runtime 失败封装，
   - `error_code="runtime.generic_error"`
   - `error_category="runtime"`
   - `hints` 通常包含 `Inspect \`error_message\` for detailed failure reason.`
3. 业务级 crypto 错误细节（如 `crypto.decrypt_failed`）当前在 `error_message` 文本中体现。
4. 进度事件不进入该 response envelope；通过独立 side channel callback 输出：
   - `tracer_core_set_crypto_progress_callback(...)`
   - payload 字段定义见 `progress_callback_v1.md`
   - callback 不改变请求/响应 JSON 契约与退出码语义。

## 7. 请求与响应示例

### 7.1 Encrypt（单文件）
请求：
```json
{
  "input_path": "C:/data/2026-01.txt",
  "output_path": "C:/out/2026-01.tracer",
  "passphrase": "phase3pass",
  "date_check_mode": "full",
  "security_level": "high"
}
```

成功响应（示意）：
```json
{
  "ok": true,
  "content": "Encrypted: C:/data/2026-01.txt -> C:/out/2026-01.tracer",
  "error_message": "",
  "error_code": "",
  "error_category": "",
  "hints": []
}
```

### 7.2 Encrypt（参数错误）
请求：
```json
{
  "input_path": "C:/data/2026-01.txt",
  "output_path": "C:/out/2026-01.tracer",
  "passphrase": "phase3pass",
  "security_level": "ultra"
}
```

失败响应（示意）：
```json
{
  "ok": false,
  "content": "",
  "error_message": "field `security_level` must be one of: min|interactive|moderate|high|max (alias: sensitive).",
  "error_code": "runtime.generic_error",
  "error_category": "runtime",
  "hints": [
    "Inspect `error_message` for detailed failure reason."
  ]
}
```

## 8. 关联文档
1. `docs/time_tracer/core/contracts/c_abi.md`
2. `docs/time_tracer/core/contracts/crypto/error_model_v1.md`
3. `docs/time_tracer/core/contracts/crypto/file_format_v2.md`
4. `docs/time_tracer/core/contracts/crypto/progress_callback_v1.md`
