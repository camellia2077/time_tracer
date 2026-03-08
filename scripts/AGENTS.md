# AGENTS Guide (scripts/devtools)

当任务涉及 `scripts/` 时，仅将其视为 `scripts/devtools/` 的作用域。

## 改动路由规则

1. 开发辅助脚本：
   - `scripts/devtools/**`
2. 开发辅助脚本文档：
   - `scripts/devtools/**/*.md`
3. Python 构建 / 编译 / clang-tidy 工具链：
   - 改 `tools/`，不要改回 `scripts/`

## 最小修改原则

1. `scripts/` 仅承载开发辅助脚本，不再承载构建主入口。
2. 若需求属于 build / verify / tidy / platform config，统一改 `tools/`。
3. 未明确要求时，不新增新的 `scripts/` 根级入口。
