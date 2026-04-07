# Core Runtime Crypto JSON Contract v1

## 状态
1. 状态：Active
2. 适用范围：`tracer_core_runtime_crypto_encrypt_json` / `decrypt_json` / `inspect_json`
3. 目标：定义 C ABI 下 crypto JSON 请求与响应的稳定语义，供 Windows CLI / 其他 host 统一接入。

## 1. 对外 ABI 入口
1. `const char* tracer_core_runtime_crypto_encrypt_json(TtCoreRuntimeHandle*, const char* request_json)`
2. `const char* tracer_core_runtime_crypto_decrypt_json(TtCoreRuntimeHandle*, const char* request_json)`
3. `const char* tracer_core_runtime_crypto_inspect_json(TtCoreRuntimeHandle*, const char* request_json)`

## 2. 通用请求规则
1. 请求必须为 UTF-8 JSON object 且不能为空。
2. 路径字段统一按 `std::filesystem::absolute` 解析为绝对路径再执行。
3. 扩展名判断大小写不敏感。
4. 必填字符串字段必须非空。
5. 未声明字段不属于稳定契约。

## 3. Encrypt 接口契约

### 3.1 请求字段
1. `input_path`（必填，string）：输入文本目录路径。
2. `output_path`（必填，string）：输出 `.tracer` 文件路径或已存在目录。
3. `passphrase`（必填，string）：口令。
4. `date_check_mode`（可选，string）：`none|continuity|full`，默认 `none`。
5. `security_level`（可选，string）：`min|interactive|moderate|high|max`，默认 `interactive`。
   - `sensitive` 兼容为 `high`。

### 3.2 处理语义
1. `input_path` 必须是已存在目录。
2. runtime 会递归收集目录下全部 `.txt` 文件；非 `.txt` 文件忽略。
3. 若目录中没有 `.txt`，encrypt 失败。
4. 每个 `.txt` 都会在导出前执行结构校验和逻辑校验；任一失败则整个导出失败。
5. 每个 `.txt` 的 basename 必须精确等于 `YYYY-MM.txt`，且文件内容头 `yYYYY + mMM` 必须与文件名一致。
6. 若多个输入 TXT 解析到同一 `YYYY-MM`，encrypt 失败。
7. runtime 会读取当前 active 的 3 个 converter TOML，并与：
   - `manifest.toml`
   - `config/converter/interval_processor_config.toml`
   - `config/converter/alias_mapping.toml`
   - `config/converter/aliases/*.toml`
   - `config/converter/duration_rules.toml`
   - `payload/<year>/YYYY-MM.txt`
   共同组装为 tracer exchange package `v4`，再交给外层 `.tracer` v2 容器压缩加密。
8. 若 `output_path` 已存在且是目录，则实际输出为：
   - `<output_path>/<input_dir_name>.tracer`
9. 否则将 `output_path` 的扩展名替换为 `.tracer` 作为最终输出文件。

### 3.3 成功 `content` 文本
1. 第一行：`Exported complete tracer exchange package: <input_abs> -> <output_abs>`
2. 第二行：`Included payload TXT files: <count>`
3. 第三行：`Included converter TOML files: 3`
4. 第四行：`Manifest included: yes`

## 4. Decrypt 接口契约

### 4.1 请求字段
1. `input_path`（必填，string）：输入 `.tracer` 文件路径。
2. `passphrase`（必填，string）：口令。
3. `output_path`（可选，string）：事务工作根目录；若提供，必须是非空字符串。
   - 用于 staging、备份与失败时保留调试现场。
   - 不是正式导入结果目录。

### 4.2 处理语义
1. `input_path` 必须是已存在的单个 `.tracer` 文件。
2. decrypt 的当前业务语义是“事务式完整导入 tracer exchange package”，不是“解包到目录”。
3. 外层 `.tracer` 解密后，runtime 会解析内层 tracer exchange package `v4`。
4. runtime 必须先成功加载校验包内：
   - `config/converter/interval_processor_config.toml`
   - `config/converter/alias_mapping.toml`
   - `config/converter/aliases/*.toml`
   - `config/converter/duration_rules.toml`
5. runtime 会备份当前 active converter config 与将被覆盖的本地月份 TXT。
6. 随后包内 3 个 converter TOML 会覆盖当前 active config。
7. runtime 会构造“包内月份覆盖 + 包外月份保留”的有效本地 TXT 视图。
8. 在新 config 下，runtime 会对全部有效 TXT 先执行结构校验，再执行逻辑校验。
9. 校验通过后，runtime 才会更新 active text root。
10. 更新 active text root 后，runtime 会基于全部有效 TXT 全量重建数据库。
11. 任一步失败都必须回滚 active converter config、本地 TXT 与数据库。
12. `output_path` 若提供，仅作为事务工作根与失败调试目录根；成功时不承诺保留可消费的解包结果目录。

### 4.3 成功 `content` 文本
1. 第一行：`Imported complete tracer exchange package: <input_abs>`
2. 第二行：`Updated active text root: <active_text_root_abs>`
3. 第三行：`Applied converter config: yes`
4. 后续摘要至少包含：
   - `Replaced months: <count>`
   - `Preserved local months: <count>`
   - `Rebuilt months: <count>`
   - `Database rebuilt: yes`
5. 若备份清理失败，可追加：
   - `Retained backup root: <backup_root_abs> | <cleanup_error>`

## 5. Inspect 接口契约

### 5.1 请求字段
1. `input_path`（必填，string）：目标 `.tracer` 文件路径。
2. `passphrase`（必填，string）：口令。

### 5.2 处理语义
1. `input_path` 必须是已存在的单个 `.tracer` 文件。
2. `output_path` 不属于 inspect 契约；若请求带有该字段，直接失败。
3. inspect 会先读取外层 `.tracer` 头部元信息，再用 `passphrase` 解密内层 package，并输出包摘要。
4. inspect 使用临时 staging 目录完成解密；命令成功或失败后都不保留这些临时文件。

### 5.3 成功 `content` 文本字段
1. `File: <path>`
2. 外层 `.tracer` 头字段：
   - `version`
   - `kdf_id`
   - `cipher_id`
   - `compression_id`
   - `compression_level`
   - `ops_limit`
   - `mem_limit_kib`
   - `plaintext_size`
   - `ciphertext_size`
3. `Package:` 段：
   - `package_type`
   - `package_version`
   - `source_root_name`
   - `payload_file_count`
   - 每个 payload 文件一行：`payload/<year>/YYYY-MM.txt: present|missing (<bytes> bytes)`
   - 3 个 converter TOML 的 present/missing 与大小

## 6. Exchange Package 校验语义
1. decrypt / inspect 成功的前提是：外层 `.tracer` 载荷必须是有效的 tracer exchange package。
2. 当前 exchange package 载荷契约见：
   - `docs/time_tracer/core/contracts/crypto/tracer_exchange_package_v4.md`
3. 若内层 package 不满足该契约，错误消息通常以：
   - `unsupported/malformed tracer package: ...`
   开头。

## 7. 通用响应契约
返回统一 JSON envelope（UTF-8）：
1. `ok`（bool）
2. `content`（string）
3. `error_message`（string）
4. `error_code`（string）
5. `error_category`（string）
6. `hints`（string[]）

说明：
1. 成功时：`ok=true`，`error_message=""`，`error_code=""`，`error_category=""`，`hints=[]`。
2. 失败时：当前这 3 个 crypto C ABI 接口统一走 runtime 失败封装：
   - `error_code="runtime.generic_error"`
   - `error_category="runtime"`
3. 业务级 crypto / package 错误细节当前在 `error_message` 文本中体现。

## 8. 请求与响应示例

### 8.1 Encrypt（目录导出）
请求：
```json
{
  "input_path": "C:/data",
  "output_path": "C:/out/test-data.tracer",
  "passphrase": "phase3pass",
  "date_check_mode": "full",
  "security_level": "high"
}
```

### 8.2 Decrypt（事务导入，默认工作根）
请求：
```json
{
  "input_path": "C:/out/test-data.tracer",
  "passphrase": "phase3pass"
}
```

### 8.3 Decrypt（事务导入，自定义工作根）
请求：
```json
{
  "input_path": "C:/out/test-data.tracer",
  "output_path": "C:/tmp/tracer_import_work",
  "passphrase": "phase3pass"
}
```

### 8.4 Inspect（外层头 + 包摘要）
请求：
```json
{
  "input_path": "C:/out/test-data.tracer",
  "passphrase": "phase3pass"
}
```

## 9. 关联文档
1. `docs/time_tracer/core/shared/c_abi.md`
2. `docs/time_tracer/core/contracts/crypto/error_model_v1.md`
3. `docs/time_tracer/core/contracts/crypto/file_format_v2.md`
4. `docs/time_tracer/core/contracts/crypto/tracer_exchange_package_v4.md`
5. `docs/time_tracer/core/contracts/crypto/progress_callback_v1.md`
