# CLI Validate Logic Guide (validate-logic)

仅执行逻辑校验（会先转换，但不会保存 JSON，也不会入库）。

---

## 用法

```
time_tracker_cli.exe validate-logic <path> [options]
```

## 参数

- `path`：源目录或单个 `.txt` 文件路径。

## 选项

- `--date-check`：日期校验模式（`continuity` / `full` / `strict` / `none` / `off`）
- `--no-date-check`：禁用日期校验

## 示例

```
time_tracker_cli.exe validate-logic C:\data\logs
time_tracker_cli.exe validate-logic C:\data\logs --date-check full
```

## 全局选项

- `-h` / `--help`
- `-v` / `--version`
