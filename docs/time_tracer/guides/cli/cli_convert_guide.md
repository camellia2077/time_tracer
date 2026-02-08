# CLI Convert Guide (convert)

将源 `.txt` 日志转换为处理后的 JSON（不入库）。

---

## 用法

```
time_tracker_cli.exe convert <path>
```

## 参数

- `path`：源文件或目录路径（递归扫描 `.txt`）。

## 行为说明

- 会按配置执行结构/逻辑校验与转换。
- 是否保存 JSON、日期校验模式等由配置默认值决定。

## 示例

```
time_tracker_cli.exe convert C:\data\logs
time_tracker_cli.exe convert C:\data\logs\2021_01.txt
```

## 全局选项

- `-h` / `--help`
- `-v` / `--version`
