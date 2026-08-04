# Clang-Tidy 任务输入

`tidy` 的默认输入是 CMake wrapper 生成的结构化结果：

```powershell
python tools/run.py tidy `
  --app tracer_core_shell `
  --source-scope core_family `
  --build-dir build_tidy_core_family
```

```text
out/tidy/<app>/<workspace>/structured_tidy_results/check_*.json
  -> tasks/clusters/<source>_<hash>/task_*.json
```

机器消费只使用 canonical `task_*.json`。`task_*.toon` 和 `task_*.log` 仍可作为阅读视图，但不能作为 task-local 命令输入。

```powershell
python tools/run.py tidy --app tracer_core_shell --source-scope core_family --build-dir build_tidy_core_family --task-view toon
```

未生成结构化结果时，`tidy` 会失败并提示恢复结构化 clang-tidy wrapper；不会解析 `build.log`。
