---
description: Windows Rust CLI 变更后的标准构建与验证流程（Rust-only）
---

## 当前范围

- 唯一 CLI 实现：`apps/tracer_cli/windows/rust_cli/`
- 运行时核心：`tracer_core.dll`（C ABI，桥接文件：`src/core/runtime.rs`）
- 旧实现已归档，不参与主线构建与测试

## 必跑流程

1. 日常提交前：
   - `python scripts/run.py post-change --app tracer_core --run-tests always --build-dir build_fast --concise`
2. 阶段验收：
   - `python scripts/run.py verify --app tracer_core --quick --scope batch --concise`
3. 结果检查：
- `apps/tracer_core_shell/build_fast/post_change_last.json`
   - `test/output/artifact_windows_cli/result.json`
   - `test/output/artifact_windows_cli/logs/output.log`
4. 必须满足：
   - `result.json` 包含 `"success": true`

## 可选分步执行

```powershell
bash apps/tracer_cli/windows/scripts/build_core_runtime_release.sh
bash apps/tracer_cli/windows/scripts/build_rust_from_windows_build.sh
python scripts/run.py verify --app tracer_core --build-dir build_fast --scope artifact --concise
```

## Agent 快速定位（按职责）

1. 全局参数与入口：
   - `apps/tracer_cli/windows/rust_cli/src/main.rs`
   - `apps/tracer_cli/windows/rust_cli/src/cli/mod.rs`
2. 命令分发：
   - `apps/tracer_cli/windows/rust_cli/src/commands/mod.rs`
   - `apps/tracer_cli/windows/rust_cli/src/commands/handler.rs`
3. 命令实现：
   - `apps/tracer_cli/windows/rust_cli/src/commands/handlers/*.rs`
4. Core C ABI 桥接：
   - `apps/tracer_cli/windows/rust_cli/src/core/runtime.rs`
5. 错误码与错误文本策略：
   - `apps/tracer_cli/windows/rust_cli/src/error/mod.rs`
6. 许可证输出：
   - `apps/tracer_cli/windows/rust_cli/src/licenses.rs`
7. 运行时模板与配置：
   - `apps/tracer_cli/windows/rust_cli/runtime/assets/`
   - `apps/tracer_cli/windows/rust_cli/runtime/config/`
8. 输出契约测试入口：
   - `test/suites/tracer_windows_rust_cli/tests/command_groups.toml`
   - `test/suites/tracer_windows_rust_cli/tests/commands_tree_version.toml`
   - `test/suites/tracer_windows_rust_cli/tests/commands_query_data.toml`
   - `test/suites/tracer_windows_rust_cli/tests/commands_crypto.toml`
9. EXE 图标生成与注入：
   - 默认 SVG 源（圆角白底）：`apps/shared/branding/sharp_rounded_white.svg`
   - 可选 SVG 源（透明底）：`apps/shared/branding/sharp_transparent.svg`
   - 构建生成：`apps/tracer_cli/windows/rust_cli/<build_dir>/resources/time_tracer_cli.ico`
   - 注入逻辑：`apps/tracer_cli/windows/rust_cli/build.rs`
   - Python 构建入口：`scripts/toolchain/commands/cmd_build/cargo.py`

## 文档优先定位（避免全局搜索）

1. 先读总索引：`docs/time_tracer/clients/windows_cli/README.md`
2. 再按改动类型选规范：
   - 结构/分层：`docs/time_tracer/clients/windows_cli/specs/STRUCTURE.md`
   - 输出/文案：`docs/time_tracer/clients/windows_cli/specs/cli-output-style.md`
   - 颜色/终端显示：`docs/time_tracer/clients/windows_cli/specs/console-color.md`
   - Core ABI 契约：`docs/time_tracer/core/contracts/c_abi.md`
3. 定位完成后再改代码，最后回写文档与测试断言（若涉及契约/文案变更）。

## 修改约束

- 使用 `scripts/run.py` 或 `apps/tracer_cli/windows/scripts/*.sh` 作为统一入口。
- 不要重新引入已归档前端的运行时依赖。
- 测试输入源固定为 `test/data`。
- 临时产物放在 `temp/`。

## 文档入口（变更后需同步）

- `docs/time_tracer/clients/windows_cli/README.md`
- `docs/time_tracer/clients/windows_cli/specs/STRUCTURE.md`
- `docs/time_tracer/clients/windows_cli/specs/cli-output-style.md`
- `docs/time_tracer/clients/windows_cli/specs/console-color.md`
- `docs/time_tracer/core/contracts/c_abi.md`
