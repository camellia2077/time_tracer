# AGENTS Guide (scripts)

当任务涉及 `scripts/` 下的 Python 构建/验证工具链时，先阅读以下文档再改代码：

1. `docs/toolchain/python_command_map.md`
2. `scripts/toolchain/README.md`

## 改动路由规则

1. 只改命令行参数定义：
   - `scripts/toolchain/cli/handlers/*.py`
2. 只改构建/验证执行逻辑：
   - `scripts/toolchain/commands/**`
3. 只改默认配置与 profile：
   - `scripts/toolchain/config.toml`
   - `scripts/toolchain/core/config.py`
4. 只改入口转发：
   - `scripts/run.py`
5. 只改 Windows CLI wrapper 参数通道：
   - `apps/tracer_cli/windows/scripts/build*.sh`

## 最小修改原则

1. 先改最靠近问题的一层，避免跨层重复修复。
2. 若改动影响参数协议，必须同步更新对应 README/文档。
3. 未明确要求时，保持向后兼容。

