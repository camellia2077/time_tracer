# Clang-Tidy Agent SOP

当前流程面向持续运行的 Agent，数据模型固定为：

```text
scan -> source cluster -> tasks
```

不存在 `tidy-batch`，也不按全局 `num` 或 batch 继续处理。

## 标准入口

```powershell
python tools/run.py tidy-agent --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family --max-clusters 3 --max-tasks 10 --max-minutes 30
```

每个 cluster 代表一个 source file 的全部当前诊断。Agent 处理完一个 cluster 后重新读取 `tasks/`，所以源码修改、归档或刷新后不会沿用旧快照。

## 单 cluster

从当前 `tasks/clusters/<source_filename>_<hash>/task_<local_id>.json` 读取对应 `.toon`，再执行：

```powershell
python tools/run.py tidy-source-step --task-log <current_task_json> --dry-run
python tools/run.py tidy-source-step --task-log <current_task_json>
```

命令内部负责安全自动修复、build sanity、focused clang-tidy re-check 和 cluster 归档。仍有诊断时，agent 直接进行最小源码修改并重新运行当前 cluster；不要等待确认，也不要手工删除 task。

## 刷新与停止

源码变化使队列过期时使用完整刷新：

```powershell
python tools/run.py tidy-refresh --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family
```

slice 达到限制是正常暂停；`blocked` 或退出码 `2` 表示 agent 需要继续修改当前源码或刷新当前 cluster；队列为空只表示可以进入 final gate。

## 完成

先预览再正式收口：

```powershell
python tools/run.py tidy-close --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family --dry-run
python tools/run.py tidy-close --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family --keep-going --concise
```

只有 final-full tidy、verify 和 queue-empty 全部通过，才能声明完成。final-full 产生的新 task 必须作为新的 cluster 继续处理。
