# Python Toolchain Guide

官方入口是 `python tools/run.py`，实现位于 `tools/toolchain/`。CLI 参数在 `cli/handlers/`，业务执行在 `commands/`，配置在 `config/` 与 `core/config.py`。

## 常用 clang-tidy 命令

```powershell
python tools/run.py tidy --app tracer_core_shell --source-scope core_family --build-dir build_tidy_core_family --task-view toon
python tools/run.py tidy-fix --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family
python tools/run.py tidy-refresh --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family
python tools/run.py tidy-agent --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family --max-clusters 3 --max-tasks 10 --max-minutes 30
python tools/run.py tidy-source-step --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family --task-log <current_task_json> --dry-run
python tools/run.py tidy-close --app tracer_core_shell --source-scope core_family --tidy-build-dir build_tidy_core_family --keep-going --concise
```

不存在 `tidy-batch`。`tidy` 只读取结构化 clang-tidy 结果。队列输出是 `tasks/clusters/<source_filename>_<hash>/task_<local_id>.*`；同一个 source file 的所有当前诊断属于同一个 cluster，task 编号只在该目录内有效。刷新、源码修改或归档后必须重新解析当前 task 路径。

## clang-tidy 输入与视图

- 唯一官方分析输入：`out/tidy/<app>/<workspace>/analysis_compile_db/compile_commands.json`。
- `task_*.json`：机器 canonical contract。
- `task_*.toon`：Agent/人类优先阅读视图。
- `task_*.log`：可选文字视图。
- `tasks/scan_manifest.json`：本次 scan 的 scope、workspace 和 generation。
- `automation/agent_run_state.json`：runner checkpoint，不替代 `tasks/`。

`tidy-agent` 是有界执行器；`paused` 表示预算到达，`blocked`/退出码 `2` 表示需要人工修改或刷新，空队列仍必须经过 `tidy-close` final gate。
