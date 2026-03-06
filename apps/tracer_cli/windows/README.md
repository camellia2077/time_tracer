# tracer_windows_rust_cli

Windows CLI 交付入口（Rust-only）。

## 目录

1. `apps/tracer_cli/windows/rust_cli/`：Rust CLI 主工程（参数解析、命令分发、Core C ABI 调用）。
2. `apps/tracer_cli/windows/scripts/`：构建与打包脚本。
3. `apps/tracer_cli/windows/agent.md`：Agent 改动后的标准验证入口。

## 常用命令

```bash
# 推荐：一次完成构建 + 集成测试
python scripts/run.py post-change --app tracer_core --run-tests always --build-dir build_fast --concise

# 阶段性验收
python scripts/run.py verify --app tracer_core --quick --scope batch --concise

# 构建 core runtime 交付包（DLL + config/assets）
bash apps/tracer_cli/windows/scripts/build_core_runtime_release.sh

# 基于 windows/build/bin 的 runtime 产物构建 Rust CLI
bash apps/tracer_cli/windows/scripts/build_rust_from_windows_build.sh

# Windows CLI 快速入口（内部调用 post-change）
bash apps/tracer_cli/windows/scripts/build_fast.sh
```

## 结果文件

- 主结果：`test/output/artifact_windows_cli/result.json`
- 用例结果：`test/output/artifact_windows_cli/result_cases.json`
- 测试总日志：`test/output/artifact_windows_cli/logs/output.log`
- post-change 记录：`apps/tracer_core_shell/build_fast/post_change_last.json`

## Runtime Config Boundary

- Canonical shared source: `assets/tracer_core/config`
- Windows generated runtime copy: `apps/tracer_cli/windows/rust_cli/runtime/config`
- 如果共享 config 语义变化，应先修改 `assets/tracer_core/config`，再通过同步流程刷新 Windows runtime copy。

## Agent 修改定位

1. CLI 入口：`apps/tracer_cli/windows/rust_cli/src/main.rs`
2. 参数模型：`apps/tracer_cli/windows/rust_cli/src/cli/mod.rs`
3. 命令分发：`apps/tracer_cli/windows/rust_cli/src/commands/mod.rs`
4. 命令处理：`apps/tracer_cli/windows/rust_cli/src/commands/handlers/*.rs`
5. Core ABI 调用：`apps/tracer_cli/windows/rust_cli/src/core/runtime.rs`
6. 错误模型：`apps/tracer_cli/windows/rust_cli/src/error/mod.rs`

## 文档驱动定位（先看文档再改代码）

1. 总入口：`docs/time_tracer/clients/windows_cli/README.md`
2. 结构/职责分层：`docs/time_tracer/clients/windows_cli/specs/STRUCTURE.md`
   - 先确定改动属于哪一层，再落到 `main.rs / cli/mod.rs / commands/handlers / core/runtime.rs`。
3. 文案与输出契约：`docs/time_tracer/clients/windows_cli/specs/cli-output-style.md`
   - 改帮助文案、错误提示、进度输出前先看这里；同步更新 `test/suites/tracer_windows_rust_cli/tests/*.toml`。
4. 颜色与终端显示：`docs/time_tracer/clients/windows_cli/specs/console-color.md`
   - 涉及 ANSI/控制台编码/显示策略时先看这里，避免破坏可读性和测试稳定性。
5. C ABI 交互：`docs/time_tracer/core/contracts/c_abi.md`
   - 改 runtime JSON 字段、错误字段或接口调用时必须同步检查该契约。

## 文档入口

- `docs/time_tracer/clients/windows_cli/README.md`
- `docs/time_tracer/clients/windows_cli/specs/STRUCTURE.md`
- `docs/time_tracer/clients/windows_cli/specs/cli-output-style.md`
- `docs/time_tracer/clients/windows_cli/specs/console-color.md`

## EXE 图标（Windows）

1. 品牌源文件（SVG）：
   - 默认（圆角白底）：`design/branding/exports/sharp_rounded_white_golden.svg`
   - 可选（透明底）：`design/branding/exports/bg_golden_vertical_padding_transparent.svg`
2. 构建时自动生成：
   - `apps/tracer_cli/windows/rust_cli/<build_dir>/resources/time_tracer_cli.ico`
3. Rust 资源注入：
   - `apps/tracer_cli/windows/rust_cli/build.rs`（读取 `TT_WINDOWS_CLI_ICON_ICO`）
4. 生成与注入入口：
   - `python scripts/run.py build --app tracer_windows_rust_cli --profile release_bundle --build-dir build`
   - `bash apps/tracer_cli/windows/scripts/build_rust_from_windows_build.sh`
5. 可选覆盖：
   - `TT_WINDOWS_CLI_ICON_ICO`：直接指定 `.ico` 路径
   - `TT_WINDOWS_CLI_ICON_SVG`：覆盖默认 `sharp_rounded_white_golden.svg`
6. 说明文档：
   - `apps/tracer_cli/windows/icon_generation.md`
