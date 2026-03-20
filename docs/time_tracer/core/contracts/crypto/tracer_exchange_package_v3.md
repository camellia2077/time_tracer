# Tracer Exchange Package v3 (`TTPKG`)

## 状态
1. 状态：Active
2. 适用范围：当前 `tracer exchange` 导出、事务式导入与 inspect 的内层明文 payload。
3. 目标：定义 `v3` package 的 manifest、严格 payload 布局与事务式导入前提。

## 1. 关系说明
1. 本文档定义的是内层 package payload，不是外层 `.tracer` 加密容器。
2. 外层 `.tracer` 容器契约仍见：
   - `docs/time_tracer/core/contracts/crypto/file_format_v2.md`
3. 当前导出链路为：
   - `manifest.toml + 3 converter TOML + payload/<year>/YYYY-MM.txt -> TTPKG v3 -> zstd -> encrypt -> .tracer`

## 2. 包内容布局
包内路径分为两部分：
1. 固定前缀（顺序固定）：
   - `manifest.toml`
   - `config/converter/interval_processor_config.toml`
   - `config/converter/alias_mapping.toml`
   - `config/converter/duration_rules.toml`
2. 变长 payload 后缀（顺序固定为字典序）：
   - `payload/<year>/YYYY-MM.txt`

约束：
1. payload 文件必须至少 1 个。
2. payload 路径必须严格位于 `payload/<year>/YYYY-MM.txt`。
3. `<year>` 目录必须与文件名中的 `YYYY` 一致。
4. payload basename 必须精确等于 `YYYY-MM.txt`。
5. payload TXT 内容头必须包含 `yYYYY + mMM`，并且与路径年月完全一致。
6. 不允许缺失、重复、额外 payload。
7. 路径编码为 UTF-8，分隔符固定使用 `/`。

## 3. `manifest.toml` 契约

### 3.1 固定字段
```toml
package_type = "tracer_exchange"
package_version = 3
producer_platform = "windows"
producer_app = "time_tracer_cli"
created_at_utc = "2026-03-21T00:00:00Z"
source_root_name = "data"

[payload]
root = "payload"
files = [
  "payload/2025/2025-01.txt",
  "payload/2026/2026-12.txt",
]

[converter]
main_config = "config/converter/interval_processor_config.toml"
alias_mapping = "config/converter/alias_mapping.toml"
duration_rules = "config/converter/duration_rules.toml"
```

### 3.2 校验规则
1. `package_type` 必须为 `tracer_exchange`。
2. `package_version` 必须为 `3`。
3. `created_at_utc` 必须为非空字符串。
4. `source_root_name` 必须为非空字符串。
5. `[payload]` 表必须存在。
6. `payload.root` 必须为 `payload`。
7. `payload.files` 必须是非空字符串数组，并满足：
   - 每一项都严格匹配 `payload/<year>/YYYY-MM.txt`
   - `<year>` 与文件名中的 `YYYY` 一致
   - 数组必须按字典序严格递增
8. `[converter]` 表必须存在，且 3 个路径值必须与固定路径完全一致。
9. `producer_platform` 与 `producer_app` 当前要求是非空字符串。

## 4. 二进制封装

### 4.1 Header（固定 32 bytes，小端序）
| offset | size | field |
| --- | --- | --- |
| 0 | 5 | `magic = "TTPKG"` |
| 5 | 1 | `package_version = 3` |
| 6 | 2 | `header_size = 32` |
| 8 | 2 | `flags = 0` |
| 10 | 2 | `entry_count = 4 + payload_file_count` |
| 12 | 2 | `manifest_index = 0` |
| 14 | 2 | `reserved_u16 = 0` |
| 16 | 4 | `toc_size_bytes` |
| 20 | 8 | `data_section_size_bytes` |
| 28 | 4 | `reserved_u32 = 0` |

### 4.2 TOC entry
每个 entry 顺序写入以下字段：
1. `u16 path_len`
2. `u16 entry_flags`
3. `u64 data_offset`
4. `u64 data_size`
5. `u8 sha256[32]`
6. `u8 path[path_len]`

### 4.3 Entry flags
1. `0x0001` = `required`
2. `0x0002` = `text`
3. 当前所有 entry 的 `entry_flags` 都必须为：
   - `0x0003` (`required | text`)

## 5. 校验规则
1. `magic` 必须为 `TTPKG`。
2. `package_version` 必须为 `3`。
3. `header_size` 必须为 `32`。
4. `flags` 必须为 `0`。
5. `entry_count` 必须至少为 `5`。
6. `manifest_index` 必须为 `0`。
7. `reserved_u16` 与 `reserved_u32` 必须为 `0`。
8. `toc_size_bytes` 与解析出的 TOC 长度必须一致。
9. `data_section_size_bytes` 必须与剩余包体长度完全匹配。
10. 前 4 个 entry 路径必须严格等于固定前缀路径。
11. 后续 payload entry 路径必须与 `manifest.payload.files` 完全一致。
12. 每个 entry 的 `entry_flags` 必须为 `0x0003`。
13. 每个 entry 的 `data_offset + data_size` 必须落在 data section 边界内。
14. 每个 entry 的 SHA-256 必须与实际数据重新计算结果完全一致。
15. `manifest.toml` 内容必须满足第 3 节契约。

## 6. 当前消费方语义
1. `inspect` 成功的前提是：外层 `.tracer` 合法，且明文 payload 满足 `v3` package 契约。
2. `decrypt/import` 的当前业务语义不再是“解包到目录”，而是事务式完整导入：
   - 校验并应用包内 3 个 converter TOML
   - 构造“包内月份覆盖 + 包外月份保留”的有效本地 TXT 视图
   - 对全部有效 TXT 先做结构校验，再做逻辑校验
   - 校验通过后更新 active text root
   - 基于全部有效 TXT 全量重建数据库
3. 任一步失败都必须回滚 active converter config、本地 TXT 与数据库状态。
4. 失败时可以保留事务工作目录作为调试现场，但不再把 package 解包目录作为正式导入产物暴露。

## 7. 错误语义
1. 任一校验失败都视为：
   - `unsupported/malformed tracer package`
2. 运行时错误消息通常采用：
   - `unsupported/malformed tracer package: <detail>`
