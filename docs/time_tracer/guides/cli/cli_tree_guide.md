# CLI Tree Guide (tree)

显示项目树结构。

---

## 用法

```
time_tracker_cli.exe tree [PROJECT_PATH] [options]
```

## 参数

- `PROJECT_PATH`（可选）：项目路径（如 `study`、`study_math`）。

## 选项

- `-r` / `--roots`：只列出所有根项目
- `-l` / `--level`：限制树深度（数字）

## 示例

```
time_tracker_cli.exe tree
time_tracker_cli.exe tree -r
time_tracker_cli.exe tree study
time_tracker_cli.exe tree -l 2
```

## 全局选项

- `-h` / `--help`
- `-v` / `--version`
