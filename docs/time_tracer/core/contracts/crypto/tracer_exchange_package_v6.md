# Tracer Exchange Package v6 (`TTPKG`)

## 状态

1. 状态：Active。
2. 适用范围：当前 tracer exchange 的逻辑 package 内容。
3. 本版本统一由 core 构造交换内容；当前外层载体为标准 ZIP AES-256
   archive，输出默认使用 `.zip` 扩展名。

## 1. 内容范围与顺序

package 的 entry 顺序固定为：

1. `manifest.toml`
2. `config/user/**` 下的全部普通文件，按统一 `/` 分隔的相对路径字典序排列
3. `payload/<year>/YYYY-MM.txt`，按路径字典序排列

`config/program/**` 永远不属于交换内容。配置文件至少包含一个普通文件；TXT payload 至少包含一个文件。所有 entry 路径必须唯一、使用 `/` 分隔，并且不能逃逸对应根目录。

## 2. `manifest.toml`

```toml
package_type = "tracer_exchange"
package_version = 6
producer_platform = "windows"
producer_app = "time_tracer_cli"
created_at_utc = "2026-07-30T00:00:00Z"
source_root_name = "data"

[config]
root = "config/user"
files = [
  "config/user/behavior.toml",
]

[payload]
root = "payload"
files = [
  "payload/2026/2026-01.txt",
]
```

规则：

1. `package_type` 必须为 `tracer_exchange`，`package_version` 必须为 `6`。
2. `[config]` 和 `[payload]` 必须存在，且各自的 `root` 必须分别为 `config/user` 与 `payload`。
3. `config.files` 必须与实际 `config/user/**` entry 集合完全一致，并按字典序严格递增。
4. `payload.files` 必须与实际 payload entry 集合完全一致，并按字典序严格递增。
5. payload 路径必须满足 `payload/<year>/YYYY-MM.txt`，目录年份、文件年份和月份必须一致。
6. payload TXT 内容继续使用现有年月头和日期校验语义。

activity hierarchy 不再是 manifest 中的特殊分支；它和 `behavior.toml` 以及 `config/user` 下新增的其他普通文件一样，通过 `config.files` 统一描述。

## 3. 外层 ZIP entry

当前磁盘文件使用 ZIP32 central directory。每个逻辑 entry 使用 UTF-8
路径、Deflate compression method，并通过 ZIP AES-256（AE-2）加密。ZIP
AES extra field 为 `0x9901`，strength 为 `3`，实际压缩方法为 `8`。

entry 路径和明文内容仍严格遵循本文件第 1、2 节；ZIP 只负责可互操作的
容器、压缩和密码保护，不改变 manifest 或业务校验语义。

## 4. 校验与消费

core 在构造和编码前校验配置根、配置清单、payload 路径、TXT 内容、entry 顺序和重复路径。解码时必须再次校验 manifest 与实际 entries 完全一致，并校验每个 entry 的边界和 SHA-256。

Core 提供两个独立阶段：

1. `BuildExportContent` 构造并校验 presentation-neutral 的 manifest 与 entries。
2. `EncodeExportContent` 产生供 core 内部校验和后续 ZIP 写出使用的逻辑
   package 表示，不依赖密码或输出路径。

`RunExport` 复用这两个阶段的结果，再直接写入 ZIP AES-256 entries。

当前外层保护策略固定为 ZIP AES-256：

1. ZIP entry 直接对应 `manifest.toml`、`config/user/**` 和 `payload/**`，
   不再把 TTPKG 作为外层密文 payload。
2. 每个 entry 使用 Deflate 方法和 ZIP AES-256（AE-2）加密。
3. ZIP AES 使用 PBKDF2-HMAC-SHA1（1000 rounds）、AES-256-CTR 和
   HMAC-SHA1 authentication code。
4. 正确密码的用户可以使用支持 ZIP AES 的普通 ZIP 工具直接查看和解包
   数据；ZIP 文件名和目录结构保持可见，entry 内容受密码保护。

当前实现暂不支持混合组合。Android 的普通 TXT+TOML 导出直接消费逻辑 entries，不进入 exchange package 保护流程；后续 Android exchange package 接入时选择第二种组合。

## 5. 错误语义

内容或 package 任一校验失败都视为 `unsupported/malformed tracer package`，并附带具体失败原因。
