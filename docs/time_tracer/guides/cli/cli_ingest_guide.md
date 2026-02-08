# CLI Ingest Guide (ingest)

执行完整流水线：结构校验 -> 转换 -> 逻辑校验 -> 入库。

---

## 用法

```
time_tracker_cli.exe ingest <path> [options]
```

## 参数

- `path`：源目录或单个 `.txt` 文件路径。

## 选项

- `--date-check`：日期校验模式（`continuity` / `full` / `strict` / `none` / `off`）
- `--no-date-check`：禁用日期校验
- `--save-processed`：保存转换后的 JSON
- `--no-save`：不保存 JSON

## 示例

```
time_tracker_cli.exe ingest C:\data\logs
time_tracker_cli.exe ingest C:\data\logs --date-check continuity
time_tracker_cli.exe ingest C:\data\logs --no-date-check --no-save
```

## 全局选项

- `-h` / `--help`
- `-v` / `--version`
