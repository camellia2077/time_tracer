# `exchange`

处理 tracer exchange 数据包，用于跨环境导出、导入和检查。

## 子命令

- `exchange export`：导出 exchange 包。
- `exchange import`：导入并替换活动配置和相关数据。
- `exchange inspect`：检查包内容，不执行导入。

```powershell
time_tracer_cli --output <package_path> exchange export --in <txt_dir> --date-check continuity
time_tracer_cli exchange inspect --in <package_path>
time_tracer_cli exchange import --in <package_path>
```

导入会刷新 Core 的运行时配置；CLI 不维护独立的配置缓存。

`exchange export` 默认不做日期完整性校验；如需校验，使用
`--date-check continuity|full`，或使用 `--no-date-check` 明确关闭。
