# AGENTS Guide (test)

本文件只保留 `test/` 范围的阅读顺序与索引；详细测试契约统一维护在 `docs/` 或 suite-local 文档中。

## 先读哪些文档

1. 根测试索引：
   - `test/README.md`
2. 工具链测试与结果契约：
   - `docs/toolchain/test/README.md`
3. 工具链 workflow 约定：
   - `docs/toolchain/workflows/README.md`
4. 测试历史：
   - `docs/toolchain/test/history/README.md`
5. suite 布局与命名：
   - `test/suites/README.md`
6. core/shell 路由与边界：
   - `docs/time_tracer/core/README.md`
   - `docs/time_tracer/core/specs/AGENT_ONBOARDING.md`
7. 产物侧验证文档：
   - `docs/time_tracer/presentation/cli/README.md`
   - `docs/time_tracer/presentation/android/README.md`

## 工作约束

1. `test/run.py` 是 test workspace 入口。
2. suite 级细节优先写在 suite-local `README.md` 或 `docs/`，不要把 `test/AGENTS.md` 扩写成厚文档。
3. 改测试入口、结果契约、artifact suite 约定时，同步更新 `docs/toolchain/test/README.md` 或对应 suite 文档。
