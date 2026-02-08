# CLI Validate Structure Guide (validate-structure)

仅验证源 TXT 的语法与结构（只读，不转换、不入库）。

---

## 用法

```
time_tracker_cli.exe validate-structure <path>
```

## 参数

- `path`：源目录或单个 `.txt` 文件路径。

## 示例

```
time_tracker_cli.exe validate-structure C:\data\logs
time_tracker_cli.exe validate-structure C:\data\logs\2021_01.txt
```

## 全局选项

- `-h` / `--help`
- `-v` / `--version`
