---
description: Windows Rust CLI 模块的标准构建与验证流程（Rust-only）
---

## 当前范围

- 唯一 CLI 实现：`apps/tracer_cli/windows/rust_cli/`
- 运行时核心：`tracer_core.dll`
- Core C ABI 桥接入口：`apps/tracer_cli/windows/rust_cli/src/core/runtime.rs`
- 历史旧实现已归档，不参与当前主线构建与测试

## 必跑流程

1. 日常改动后的标准验证：
   - `python scripts/run.py post-change --app tracer_core --run-tests always --build-dir build_fast --concise`
2. 阶段性批量验收：
   - `python scripts/run.py verify --app tracer_core --quick --scope batch --concise`
3. 结果检查：
   - `apps/tracer_core_shell/build_fast/post_change_last.json`
   - `test/output/artifact_windows_cli/result.json`
   - `test/output/artifact_windows_cli/logs/output.log`
4. 通过标准：
   - `test/output/artifact_windows_cli/result.json` 中应包含 `"success": true`

## 可选分步执行

```powershell
bash apps/tracer_cli/windows/scripts/build_core_runtime_release.sh
bash apps/tracer_cli/windows/scripts/build_rust_from_windows_build.sh
python scripts/run.py verify --app tracer_core --build-dir build_fast --scope artifact --concise
```

## Agent 快速定位

1. CLI 入口与参数解析：
   - `apps/tracer_cli/windows/rust_cli/src/main.rs`
   - `apps/tracer_cli/windows/rust_cli/src/cli/mod.rs`
2. 命令分发：
   - `apps/tracer_cli/windows/rust_cli/src/commands/mod.rs`
   - `apps/tracer_cli/windows/rust_cli/src/commands/handler.rs`
3. 命令实现：
   - `apps/tracer_cli/windows/rust_cli/src/commands/handlers/*.rs`
4. Core C ABI 桥接：
   - `apps/tracer_cli/windows/rust_cli/src/core/runtime.rs`
5. 错误模型与错误文案：
   - `apps/tracer_cli/windows/rust_cli/src/error/mod.rs`
6. 许可证输出：
   - `apps/tracer_cli/windows/rust_cli/src/licenses.rs`
7. 运行时模板与配置：
   - `apps/tracer_cli/windows/rust_cli/runtime/assets/`
   - `apps/tracer_cli/windows/rust_cli/runtime/config/`
   - canonical source：`assets/tracer_core/config`
   - `runtime/config/` 是 generated runtime copy，不是共享配置源头
8. 输出契约测试入口：
   - `test/suites/tracer_windows_rust_cli/tests/command_groups.toml`
   - `test/suites/tracer_windows_rust_cli/tests/commands_tree_version.toml`
   - `test/suites/tracer_windows_rust_cli/tests/commands_query_data.toml`
   - `test/suites/tracer_windows_rust_cli/tests/commands_crypto.toml`
9. EXE 图标生成与注入：
   - 默认 SVG 源：`design/branding/exports/bg_indigo_mist_vertical_padding.svg`
   - 可选透明底 SVG：`design/branding/exports/bg_golden_vertical_padding_transparent.svg`
   - 构建生成：`apps/tracer_cli/windows/rust_cli/<build_dir>/resources/time_tracer_cli.ico`
   - 注入逻辑：`apps/tracer_cli/windows/rust_cli/build.rs`
   - 默认 SVG 解析逻辑：`scripts/toolchain/commands/cmd_build/windows_icon_resources.py`
   - 说明文档：`apps/tracer_cli/windows/icon_generation.md`

## 文档优先定位

修改代码前，优先阅读以下文档：

1. 总索引：
   - `docs/time_tracer/clients/windows_cli/README.md`
2. 结构与分层：
   - `docs/time_tracer/clients/windows_cli/specs/STRUCTURE.md`
3. 输出与文案规范：
   - `docs/time_tracer/clients/windows_cli/specs/cli-output-style.md`
4. 颜色与终端显示：
   - `docs/time_tracer/clients/windows_cli/specs/console-color.md`
5. Core ABI 契约：
   - `docs/time_tracer/core/contracts/c_abi.md`

如果改动涉及命令输出、帮助文案、错误提示或契约字段，修改代码后要同步回写文档与测试断言。

## 修改约束

- 使用 `scripts/run.py` 或 `apps/tracer_cli/windows/scripts/*.sh` 作为统一入口
- 不要重新引入已归档前端实现的运行时依赖
- 测试输入源固定为 `test/data`
- 临时产物放在 `temp/`

## 文档入口

- `docs/time_tracer/clients/windows_cli/README.md`
- `docs/time_tracer/clients/windows_cli/specs/STRUCTURE.md`
- `docs/time_tracer/clients/windows_cli/specs/cli-output-style.md`
- `docs/time_tracer/clients/windows_cli/specs/console-color.md`
- `docs/time_tracer/core/contracts/c_abi.md`
