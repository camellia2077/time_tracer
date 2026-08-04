# Clang-Tidy 工作流与状态

本文档描述当前唯一的 clang-tidy 队列模型：`scan -> source clusters -> tasks`。旧的 batch checkpoint、batch state 和按 batch 增量 refresh 已删除。

## 1. 工作区

`core_family` 使用：

```text
out/tidy/tracer_core_shell/build_tidy_core_family/
  build.log
  analysis_compile_db/compile_commands.json
  structured_tidy_results/
    check_*.json
  tasks/
    clusters/
      <source_filename>_<hash>/
        cluster.json
        task_001.json
        task_001.toon|log
    scan_manifest.json
    queue_state.json
  tasks/archive/<source_filename>_<hash>/
  automation/
  tidy_result.json
  tidy_state.json
```

`core_family` 当前覆盖 `libs/tracer_core/src`、`libs/tracer_adapters_io/src`、`libs/tracer_core_bridge_common/src` 和 `libs/tracer_transport/src`。它不自动包含 `apps/tracer_core_shell`。

## 2. 生成与重建队列

```powershell
python tools/run.py tidy --app tracer_core_shell --source-scope core_family --build-dir build_tidy_core_family --task-view toon
```

`tidy` 只读取 CMake 注入的结构化 clang-tidy wrapper 结果。wrapper 为每个源文件写入 `structured_tidy_results/check_*.json`，然后 Python 聚合器直接生成当前 `tasks/clusters/<cluster>/task_*.json`；历史 `tasks/archive/` 不参与清理。按 `source_file` 聚合 cluster，再在每个 cluster 目录内从 `task_001` 开始编号。

`tidy-refresh` 始终重新执行完整 scan -> cluster -> task 队列生成，用于源码变化后丢弃过期快照：

```powershell
python tools/run.py tidy-refresh --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family
```

刷新后不能复用旧 task 路径、行号或诊断文本；必须重新从 `tasks/` 解析。

## 3. Agent 处理

```powershell
python tools/run.py tidy-agent --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family --max-clusters 3 --max-tasks 10 --max-minutes 30
```

一次 Agent slice 受 cluster 数、task 数和分钟数的最小值限制。它处理一个 cluster 后重新扫描队列，因此前一个 cluster 的修改不会让后续 Agent 继续消费过时快照。

单 cluster 路径：

```powershell
python tools/run.py tidy-source-step --task-log <current_task_json> --dry-run
python tools/run.py tidy-source-step --task-log <current_task_json>
```

该命令会扩展到同一 source file 的全部 pending task，执行安全自动修复、build sanity 和 focused clang-tidy re-check。只有 re-check 清洁时，才将整个 cluster 移到 `tasks/archive/<source_filename>_<hash>/`。

## 4. 停止条件

- `paused/slice_limit`：正常到达本轮预算，直接用 `tidy-agent` 继续。
- `blocked` 或退出码 `2`：当前 cluster 仍有诊断，agent 应直接修改源码并重新运行当前 cluster；不是等待用户确认，也不是完成。
- `failed`：命令或验证失败，先查看 automation 和 re-check 日志。
- `queue_empty_requires_tidy_close`：当前 queue 暂时为空，必须走 final gate。

最终完成必须同时满足：`tasks/` 下无任何 task artifact、`tidy-close` 返回 `0`，且 `tidy_result.json.final_gate` 中 final-full tidy、verify、queue-empty 都通过。

## 5. Final gate

```powershell
python tools/run.py tidy-close --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family --dry-run
python tools/run.py tidy-close --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family --keep-going --concise
```

final-full 若重新生成 task，`tidy-close` 返回 `pending_after_final_full`；回到 `tidy-agent` 处理新 cluster，不能把旧 verify 结果当作完成证明。`--tidy-only` 只跳过 verify，不改变 queue-empty 要求。
