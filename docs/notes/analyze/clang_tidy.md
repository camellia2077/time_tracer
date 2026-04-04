# Clang-Tidy 起手说明

`clang-tidy` 现在统一通过 Python toolchain 入口启动，不再依赖
`apps/tracer_core_shell/scripts/*.sh` 转发脚本。

统一入口：

```bash
python tools/run.py ...
```

## tracer_core_shell / core_family

当前常用默认组合：

- app: `tracer_core_shell`
- source scope: `core_family`
- tidy workspace: `build_tidy_core_family`

## 配置档选择

### 默认日常版

不传额外参数时，默认使用仓库根目录的：

- `.clang-tidy`

也可以显式写成：

```bash
--config-file .clang-tidy
```

### 可选严格版

严格版使用仓库根目录的：

- `.clang-tidy.strict`

推荐直接传：

```bash
--strict-config
```

等价写法：

```bash
--config-file .clang-tidy.strict
```

## 开始一次新的 clang-tidy 队列

默认日常版：

```bash
python tools/run.py tidy --app tracer_core_shell --source-scope core_family --build-dir build_tidy_core_family
```

严格版：

```bash
python tools/run.py tidy --app tracer_core_shell --source-scope core_family --build-dir build_tidy_core_family --strict-config
```

这条命令会重新执行 `clang-tidy`，并生成当前队列。

## 仅从已有 build.log 重新切分

如果你不想重新跑一遍 `clang-tidy`，只是想从已有 `build.log` 重新生成 task 队列，用：

```bash
python tools/run.py tidy-split --app tracer_core_shell --source-scope core_family --build-dir build_tidy_core_family
```

注意：

- `tidy-split` 不会重新执行 `clang-tidy`
- 所以这里不涉及日常版 / 严格版切换

## 可选的 task 视图

### `tidy`

`tidy` 可以额外指定要不要输出阅读视图：

```bash
python tools/run.py tidy --app tracer_core_shell --source-scope core_family --build-dir build_tidy_core_family --task-view json
python tools/run.py tidy --app tracer_core_shell --source-scope core_family --build-dir build_tidy_core_family --task-view toon
python tools/run.py tidy --app tracer_core_shell --source-scope core_family --build-dir build_tidy_core_family --task-view text
python tools/run.py tidy --app tracer_core_shell --source-scope core_family --build-dir build_tidy_core_family --task-view text+toon
```

### `tidy-split`

`tidy-split` 显式传 `--task-view` 时，只允许面向人阅读的视图：

```bash
python tools/run.py tidy-split --app tracer_core_shell --source-scope core_family --build-dir build_tidy_core_family --task-view text
python tools/run.py tidy-split --app tracer_core_shell --source-scope core_family --build-dir build_tidy_core_family --task-view text+toon
```

不要传：

```bash
python tools/run.py tidy-split --app tracer_core_shell --source-scope core_family --build-dir build_tidy_core_family --task-view json
python tools/run.py tidy-split --app tracer_core_shell --source-scope core_family --build-dir build_tidy_core_family --task-view toon
```

上面两种会被 CLI 直接拒绝。

## 常见产物

- `task_*.json`
- `task_*.log`
- `task_*.toon`

如果你只是想开始一次 clang-tidy，通常只需要记住两件事：

1. 新跑一轮就用 `tidy`
2. 只想从已有 `build.log` 重切队列就用 `tidy-split`
