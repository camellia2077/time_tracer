# tracer_windows_rust_cli

Windows CLI 当前交付实现为 Rust-only 版本。

## 目录

1. `apps/tracer_cli/windows/rust_cli/`
   - Rust CLI 主工程
   - 负责参数解析、命令分发、Core C ABI 调用、运行时装配
2. `apps/tracer_cli/windows/scripts/`
   - Windows CLI 构建与打包脚本
   - 默认发布入口是 `build_windows_release.sh`
3. `apps/tracer_cli/windows/agent.md`
   - 本模块的 agent 约束与验证入口

## 常用命令

```powershell
# 推荐：代码改动后的单命令 build + 集成验证
python scripts/run.py post-change --app tracer_core --run-tests always --build-dir build_fast --concise

# 阶段性批量验证
python scripts/run.py verify --app tracer_core --quick --scope batch --concise

# 默认发布构建入口：先编 Windows runtime，再编 Rust CLI
bash apps/tracer_cli/windows/scripts/build_windows_release.sh

# 仅构建 Windows runtime 交付物（DLL + runtime layout）
bash apps/tracer_cli/windows/scripts/build_core_runtime_release.sh

# 基于已有 runtime 产物单独构建 Rust CLI
bash apps/tracer_cli/windows/scripts/build_rust_from_windows_build.sh

# Windows CLI 快速入口脚本（面向改动后的构建 + 测试）
bash apps/tracer_cli/windows/scripts/build_fast.sh
```

## 结果文件

- 主结果：`test/output/artifact_windows_cli/result.json`
- 用例结果：`test/output/artifact_windows_cli/result_cases.json`
- 聚合日志：`test/output/artifact_windows_cli/logs/output.log`
- `post-change` 状态：`apps/tracer_core_shell/build_fast/post_change_last.json`

## Runtime Config Boundary

- Canonical shared source：`assets/tracer_core/config`
- Windows generated runtime copy：`apps/tracer_cli/windows/rust_cli/runtime/config`

如果共享配置语义变更，应先修改 `assets/tracer_core/config`，再通过构建同步流程刷新 Windows runtime copy。

## Ingest Persistence Boundary

- `ingest` 只有在全部校验通过后，才允许创建数据库并写入
- 如果运行前数据库不存在，则失败的 `ingest` 不得留下新的 `.sqlite3` / `-wal` / `-shm` / `-journal`
- 不要在 Windows CLI 侧重新引入“失败后清理空库文件”的兼容补丁；长期语义由 core/runtime 边界保证

## Agent 修改定位

1. CLI 入口
   - `apps/tracer_cli/windows/rust_cli/src/main.rs`
2. 参数模型
   - `apps/tracer_cli/windows/rust_cli/src/cli/mod.rs`
3. 命令分发
   - `apps/tracer_cli/windows/rust_cli/src/commands/mod.rs`
4. 命令处理
   - `apps/tracer_cli/windows/rust_cli/src/commands/handlers/*.rs`
5. Core ABI 调用
   - `apps/tracer_cli/windows/rust_cli/src/core/runtime.rs`
6. 错误模型
   - `apps/tracer_cli/windows/rust_cli/src/error/mod.rs`

## 文档驱动定位

修改代码前，优先查看这些文档：

1. 总入口
   - `docs/time_tracer/clients/windows_cli/README.md`
2. 结构与职责分层
   - `docs/time_tracer/clients/windows_cli/specs/STRUCTURE.md`
3. CLI 输出与文案约束
   - `docs/time_tracer/clients/windows_cli/specs/cli-output-style.md`
4. 颜色与终端显示
   - `docs/time_tracer/clients/windows_cli/specs/console-color.md`
5. Core C ABI 契约
   - `docs/time_tracer/core/contracts/c_abi.md`

## 文档入口

- `docs/time_tracer/clients/windows_cli/README.md`
- `docs/time_tracer/clients/windows_cli/specs/STRUCTURE.md`
- `docs/time_tracer/clients/windows_cli/specs/cli-output-style.md`
- `docs/time_tracer/clients/windows_cli/specs/console-color.md`

## EXE 图标

1. 品牌源文件（SVG）
   - 默认：`design/branding/exports/bg_indigo_mist_vertical_padding.svg`
   - 可选透明底：`design/branding/exports/bg_golden_vertical_padding_transparent.svg`
2. 构建时自动生成
   - `apps/tracer_cli/windows/rust_cli/<build_dir>/resources/time_tracer_cli.ico`
3. Rust 资源注入
   - `apps/tracer_cli/windows/rust_cli/build.rs`
4. 构建入口
   - `python scripts/run.py build --app tracer_windows_rust_cli --profile release_bundle --build-dir build`
   - `bash apps/tracer_cli/windows/scripts/build_windows_release.sh`
   - `bash apps/tracer_cli/windows/scripts/build_rust_from_windows_build.sh`
5. 可选覆盖
   - `TT_WINDOWS_CLI_ICON_ICO`：直接指定 `.ico` 路径
   - `TT_WINDOWS_CLI_ICON_SVG`：覆盖默认 `bg_indigo_mist_vertical_padding.svg`
6. 说明文档
   - `apps/tracer_cli/windows/icon_generation.md`
   - `design/branding/windows-cli-icon-svg-application.md`
