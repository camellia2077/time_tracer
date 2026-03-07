# windows/scripts

保留 4 个脚本：

- `build_fast.sh`
- `build_windows_release.sh`
- `build_core_runtime_release.sh`
- `build_rust_from_windows_build.sh`

常用命令：

```bash
# 日常单命令入口：先编 core runtime，再编 Rust CLI
bash apps/tracer_cli/windows/scripts/build_windows_release.sh

# 代码改动后的 build + test 快速入口
bash apps/tracer_cli/windows/scripts/build_fast.sh

# 仅构建 core runtime DLL（`tracer_core.dll` + `reports_shared.dll`）
bash apps/tracer_cli/windows/scripts/build_core_runtime_release.sh

# 基于已有 runtime 产物单独构建 Rust CLI
bash apps/tracer_cli/windows/scripts/build_rust_from_windows_build.sh
```

推荐约定：

- 日常发布构建默认使用 `build_windows_release.sh`
- 只有在明确知道 core 产物已是最新时，才单独运行 `build_rust_from_windows_build.sh`
