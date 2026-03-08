# AGENTS Guide (tools)

当任务涉及 `tools/` 下的 Python 构建/验证工具链时，先阅读以下文档再改代码：

1. `docs/toolchain/clang_tidy_architecture.md`（先看文件分层与改动路由）
2. `docs/toolchain/clang_tidy_flow.md`（再看 clang-tidy 工作流与状态文件）
3. `docs/toolchain/python_command_map.md`
4. `tools/toolchain/README.md`

## 改动路由规则

1. 只改命令行参数定义：
   - `tools/toolchain/cli/handlers/*.py`
2. 只改构建/验证执行逻辑：
   - `tools/toolchain/commands/**`
3. 只改默认配置与 profile：
   - `tools/toolchain/config/*.toml`
   - `tools/toolchain/core/config.py`
4. 只改入口转发：
   - `tools/run.py`
5. 只改 Windows CLI wrapper 参数通道：
   - `apps/tracer_cli/windows/scripts/build*.sh`

## 最小修改原则

1. 先改最靠近问题的一层，避免跨层重复修复。
2. 若改动影响参数协议，必须同步更新对应 README/文档。
3. 未明确要求时，保持 `tools/` 作为唯一官方入口，不再回退旧 `scripts/` 路径。
