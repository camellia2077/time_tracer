# Clang-Tidy 工具链架构

## 核心模型

重构 tidy 代码时，必须同时遵守
[refactoring_guidance.md](refactoring_guidance.md)；本文档负责结构地图，
重构约束和验收标准由该文档定义。

工具链把 clang-tidy 输出解析为可重建的 source-cluster queue：

```text
clang-tidy -> structured_tidy_results/check_*.json
        |
        v
按 source_file 聚合
        |
        v
tasks/clusters/<source_filename>_<hash>/cluster.json + task_*.json
        |
        v
tidy-agent / tidy-source-step
        |
        v
fresh focused re-check -> tasks/archive/<source_filename>_<hash>
```

一个 cluster 是一个 source file 的全部当前 pending diagnostics；cluster 内的 task 从 `task_001` 开始局部编号。task 不是稳定身份，只是某次 scan 的诊断快照。`scan_id` 和 `queue_generation` 用来拒绝刷新前的 stale task。

## 分层

- CLI：`tools/toolchain/cli/handlers/tidy/`
- 命令：`tools/toolchain/commands/tidy/`
- Clang 底层适配：`tools/toolchain/commands/clang/`
- 扫描与输入：`tools/toolchain/commands/tidy/scan/`
- 队列与任务：`tools/toolchain/commands/tidy/queue/`
- 执行与门禁：`tools/toolchain/commands/tidy/execution/`
- 终态：`tools/toolchain/services/tidy_state.py`
- 路径：`tools/toolchain/core/generated_paths.py`

关键组件：

- `commands/clang/tidy/invocation.py`：执行单次 clang-tidy invocation 并写结构化结果。
- `commands/clang/tidy/diagnostics.py`：解析 clang-tidy diagnostics。
- `commands/clang/tidy/compile_db.py` / `config.py`：提供编译数据库和 clang-tidy 配置适配。
- `commands/clang/clangd/`：提供 clangd LSP client 和 workspace edit 适配。
- `structured_results.py`：读取结构化结果、按源码聚合、写 cluster-first queue。
- `queue/source_cluster.py`：从当前 task 路径解析完整 cluster。
- `execution/agent_run.py`：按 cluster/task/time 限制驱动 Agent，并在每个 cluster 后重解析。
- `execution/source_step.py`：处理一个 cluster，完成 focused re-check 后归档。
- `execution/step_internal/source_step_artifacts.py`：刷新 focused re-check 后的 cluster task 视图。
- `execution/step_internal/source_step_state.py`：持久化 tidy result、全局 state 和 source-cluster checkpoint。
- `execution/refresh.py`：完整重建 scan -> cluster -> task 队列。
- `execution/close.py`：执行 final-full、verify 和空队列门禁。
- `autofix/engines/text_edit_engine.py`：负责文本文件读取、编辑合并、冲突过滤和写回。
- `autofix/engines/text_edit_planner.py`：只负责根据单个 FixIntent 生成安全的同文件编辑，不直接读写工作区。

`commands/tidy/command.py` 只保留公开的 `TidyCommand` 入口；扫描、计时和结构化结果的纯转发不再通过 command wrapper，统一由 `command_execute.py` 直接编排 `scan/` 服务。

## 文件契约

```text
out/tidy/<app>/<workspace>/
  tasks/clusters/<source_filename>_<hash>/cluster.json
  tasks/clusters/<source_filename>_<hash>/task_<local_id>.json
  tasks/clusters/<source_filename>_<hash>/task_<local_id>.toon|log
  tasks/scan_manifest.json
  tasks/queue_state.json
  tasks/archive/<source_filename>_<hash>/
  tidy_result.json
  tidy_state.json
```

JSON 是唯一机器入口，TOON 是默认阅读入口，LOG 是可选阅读视图。所有 task-local 命令都必须使用当前 scan 的 JSON 路径；不再兼容以 TOON/LOG 作为 task 输入。

## 状态与门禁

`automation/agent_run_state.json` 只保存有界 runner 的暂停、阻塞和进度信息，不是 queue source of truth。真实队列永远是当前 `tasks/` 树。

`tidy-close` 的正式通过条件：

1. final-full tidy 成功；
2. verify 成功且晚于本轮最后源码变更；
3. 当前 `tasks/` 没有 task artifact。

final-full 新生成 task 时返回 `pending_after_final_full`，必须重新处理 cluster。没有单独的 batch checkpoint、batch state 或 batch transition。
