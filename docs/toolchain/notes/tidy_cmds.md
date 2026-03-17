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

## task 视图格式

### JSON only

```bash
python tools/run.py tidy --app tracer_core_shell --source-scope core_family --build-dir build_tidy_core_family --task-view json
python tools/run.py tidy-split --app tracer_core_shell --source-scope core_family --build-dir build_tidy_core_family --task-view json
```

产物：

- `task_*.json`

### Text

```bash
python tools/run.py tidy-split --app tracer_core_shell --source-scope core_family --build-dir build_tidy_core_family --task-view text
```

产物：

- `task_*.log`

### TOON

```bash
python tools/run.py tidy-split --app tracer_core_shell --source-scope core_family --build-dir build_tidy_core_family --task-view toon
```

产物：

- `task_*.toon`

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
python tools/run.py tidy-task-patch --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family --batch-id <BATCH_ID> --task-id <TASK_ID>
python tools/run.py tidy-task-fix --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family --batch-id <BATCH_ID> --task-id <TASK_ID> --dry-run
python tools/run.py tidy-task-suggest --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family --batch-id <BATCH_ID> --task-id <TASK_ID>
python tools/run.py tidy-step --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family --batch-id <BATCH_ID> --task-id <TASK_ID> --dry-run
```

## 当前 task 契约

- canonical task record: `task_*.json`
- human-readable text view: `task_*.log`
- agent-facing compact view: `task_*.toon`

后两者都是渲染视图，不再作为后续工具链的解析来源。
