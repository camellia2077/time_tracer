# Tracer Exchange Package v6 (`TTPKG`)

## 状态

1. 状态：Active。
2. 适用范围：当前 tracer exchange 的内层明文 package。
3. 本版本统一由 core 构造交换内容；外层压缩、加密和输出目标不属于本契约。

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
  "config/user/activity_hierarchy/_system.toml",
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

## 3. 二进制封装

TTPKG header 仍使用当前二进制机制：`magic = "TTPKG"`、header version `3`、固定 32 字节 header、manifest index 为 `0`。业务级 `manifest.package_version = 6` 与二进制 header version 是两个独立版本号。

每个 TOC entry 依次包含：`u16 path_len`、`u16 entry_flags`、`u64 data_offset`、`u64 data_size`、`u8 sha256[32]` 和 UTF-8 path。

entry flags：

1. `0x0001`：required。
2. `0x0002`：text。
3. `manifest.toml`、`.toml` 和 `.txt` 使用 `0x0003`；其他普通文件使用 required-only。

## 4. 校验与消费

core 在构造和编码前校验配置根、配置清单、payload 路径、TXT 内容、entry 顺序和重复路径。解码时必须再次校验 manifest 与实际 entries 完全一致，并校验每个 entry 的边界和 SHA-256。

Core 提供两个独立阶段：

1. `BuildExportContent` 构造并校验 presentation-neutral 的 manifest 与 entries。
2. `EncodeExportContent` 将逻辑内容编码为 TTPKG 字节，不依赖密码、压缩算法或输出路径。

`RunExport` 复用这两个阶段的结果，再交给现有压缩、加密和输出流程。不同 presentation 后续可以直接消费同一组逻辑 entries，自行负责其具体写出方式。

阶段四的保护策略提供两个可选组合：

1. `compression = None`、`encryption = None`：输出未保护的 TTPKG 字节。
2. `compression = Existing`、`encryption = Existing`：使用当前 CLI 的压缩+加密实现。

当前实现暂不支持混合组合。Android 的普通 TXT+TOML 导出直接消费逻辑 entries，不进入 exchange package 保护流程；后续 Android exchange package 接入时选择第二种组合。

## 5. 错误语义

内容或 package 任一校验失败都视为 `unsupported/malformed tracer package`，并附带具体失败原因。
