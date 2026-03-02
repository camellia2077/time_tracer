# windows/scripts

仅保留 3 个脚本：

- `build_fast.sh`
- `build_core_runtime_release.sh`
- `build_rust_from_windows_build.sh`

常用命令：

```bash
# 日常单命令（按 .agent/rules/AGENTS.md）：build + test
bash apps/tracer_cli/windows/scripts/build_fast.sh

# 仅构建 core runtime DLL（tracer_core.dll + reports_shared.dll）
bash apps/tracer_cli/windows/scripts/build_core_runtime_release.sh

# 从 apps/tracer_cli/windows/build/bin 获取 runtime 依赖后编译 Rust CLI
bash apps/tracer_cli/windows/scripts/build_rust_from_windows_build.sh
```

