# `pipeline`

处理 TXT 和配置：校验结构、校验业务逻辑、转换、导入和入库。

## 子命令

- `pipeline validate structure <path>`：检查 TXT 结构。
- `pipeline validate logic <path>`：检查业务规则。
- `pipeline validate all <path>`：执行完整 TXT 校验。
- `pipeline validate bundle --txt <dir> --config <dir>`：校验外部 TXT/config/user，默认不写入用户数据库；程序资源由 CLI 自身提供，不属于外部交换数据。
- `pipeline convert`：将输入转换为 Core 支持的结果。
- `pipeline import`：导入处理后的数据。
- `pipeline ingest`：读取 TXT 并写入数据库。

```powershell
time_tracer_cli pipeline validate bundle --txt <txt_dir> --config <config_dir> --no-date-check
time_tracer_cli --db <db_path> pipeline ingest <txt_dir>
```

`validate bundle` 适合排查 TXT 与 `config/user/**` 是否匹配；它使用临时运行空间，不修改输入目录或用户数据库。`<config_dir>` 只需要包含 `user/`，不需要包含 `program/`。
