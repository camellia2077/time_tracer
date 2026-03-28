# Clang-Tidy Python Commands

clang-tidy 不再依赖 `apps/tracer_core_shell/scripts/*.sh` 转发脚本。

统一入口：

```bash
python tools/run.py ...
```

## tracer_core_shell / core_family

默认 scoped workspace:

- app: `tracer_core_shell`
- source scope: `core_family`
- tidy workspace: `build_tidy_core_family`

## 生成 clang-tidy 队列

```bash
python tools/run.py tidy --app tracer_core_shell --source-scope core_family --build-dir build_tidy_core_family
```

## 仅从已有 build.log 重新切分

```bash
python tools/run.py tidy-split --app tracer_core_shell --source-scope core_family --build-dir build_tidy_core_family
```

显式传 `--task-view` 时，当前仅允许：

```bash
python tools/run.py tidy-split --app tracer_core_shell --source-scope core_family --build-dir build_tidy_core_family --task-view text
python tools/run.py tidy-split --app tracer_core_shell --source-scope core_family --build-dir build_tidy_core_family --task-view text+toon
```

不要传：

```bash
python tools/run.py tidy-split --app tracer_core_shell --source-scope core_family --build-dir build_tidy_core_family --task-view json
python tools/run.py tidy-split --app tracer_core_shell --source-scope core_family --build-dir build_tidy_core_family --task-view toon
```

上面两种现在会被 CLI 直接拒绝。

## task 视图格式

### `tidy`

`tidy` 仍然支持完整的 task view 选择：

```bash
python tools/run.py tidy --app tracer_core_shell --source-scope core_family --build-dir build_tidy_core_family --task-view json
python tools/run.py tidy --app tracer_core_shell --source-scope core_family --build-dir build_tidy_core_family --task-view toon
python tools/run.py tidy --app tracer_core_shell --source-scope core_family --build-dir build_tidy_core_family --task-view text
python tools/run.py tidy --app tracer_core_shell --source-scope core_family --build-dir build_tidy_core_family --task-view text+toon
```

### `tidy-split`

`tidy-split` 显式传参时只允许面向人阅读的视图参数：

```bash
python tools/run.py tidy-split --app tracer_core_shell --source-scope core_family --build-dir build_tidy_core_family --task-view text
python tools/run.py tidy-split --app tracer_core_shell --source-scope core_family --build-dir build_tidy_core_family --task-view text+toon
```

原因是 `tidy-split` 的机器契约固定是 canonical `task_*.json`，显式 `--task-view` 只控制是否额外渲染 `.log` / `.toon` 这类阅读视图。

### JSON canonical record

- `task_*.json`

canonical JSON 始终作为后续工具链的稳定输入。

### Text view

```bash
python tools/run.py tidy-split --app tracer_core_shell --source-scope core_family --build-dir build_tidy_core_family --task-view text
```

产物：

- `task_*.log`

### TOON view

`tidy` 可以显式要求额外输出 TOON：

```bash
python tools/run.py tidy --app tracer_core_shell --source-scope core_family --build-dir build_tidy_core_family --task-view toon
```

产物：

- `task_*.toon`
- `task_*.json`

### Text + TOON

```bash
python tools/run.py tidy --app tracer_core_shell --source-scope core_family --build-dir build_tidy_core_family --task-view text+toon
python tools/run.py tidy-split --app tracer_core_shell --source-scope core_family --build-dir build_tidy_core_family --task-view text+toon
```

产物：

- `task_*.json`
- `task_*.log`
- `task_*.toon`

## 后续常用命令

```bash
python tools/run.py tidy-fix --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family
python tools/run.py tidy-batch --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family --batch-id <BATCH_ID> --preset sop
python tools/run.py tidy-close --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family --keep-going --concise
python tools/run.py tidy-task-patch --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family --task-log <resolved_task_json>
python tools/run.py tidy-task-fix --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family --task-log <resolved_task_json> --dry-run
python tools/run.py tidy-task-suggest --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family --task-log <resolved_task_json>
python tools/run.py tidy-step --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family --task-log <resolved_task_json> --dry-run
```

## 当前 task 契约

- canonical task record: `task_*.json`
- human-readable text view: `task_*.log`
- agent-facing compact view: `task_*.toon`

后两者都是渲染视图，不再作为后续工具链的解析来源。

## 为什么 auto-fix 统一解析 JSON

- `auto-fix`、`refresh`、`clean` 这类机器消费路径现在统一以 `task_*.json` 为准。
- TOON 适合人和 agent 阅读，但它目前没有官方标准库，也没有成熟、通用的第三方解析生态。
- 如果把 TOON 当成机器契约，通常只能依赖项目内 codec 或手写解析；一旦展示格式继续演进，解析逻辑就更容易变脆。
- JSON 则直接有稳定解析器，字段结构也更适合作为 source of truth。
- 所以当前取舍是：`toon` 用来读，`json` 用来解析。
