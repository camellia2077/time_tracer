# Windows CLI 文档域

本目录用于承载 Windows CLI 展现层相关文档。

## 范围
1. 命令行命令与参数语义。
2. CLI 输出格式、交互规范、退出码策略。
3. Windows 交付与打包相关说明。

## 对应代码
1. `apps/tracer_windows_cli/`
2. `apps/tracer_windows_cli/src/api/cli/`

## 测试套件约定
1. `tracer_windows_cli` 套件是 `core + windows cli` 集成套件。
2. 推荐验证命令：`python scripts/verify.py --app time_tracer --quick`。
3. 结果输出目录：`test/output/tracer_windows_cli/`（含 `result.json`、`result_cases.json`、`logs/`）。
4. runtime guard 复用入口：`python test/runtime_guard.py --build-dir build_fast`（或 `test/run_runtime_guard.bat --build-dir build_fast`）。

## Phase 5 验收记录（2026-02-22）
1. `python scripts/verify.py --app time_tracer --quick` 已通过。
2. `release_safe` 体积对比完成：`time_tracer_cli.exe` 与关键段 `.text/.rdata` 相对基线无增量（`delta=0`）。
3. 手工冒烟已覆盖：`--version`、`--help`、缺 core dll、缺 config、典型 `query/export`。
4. 异常场景统一由 `Runtime check failed: ...` 报错，便于定位缺失运行时文件。

## 原则与规范
1. 输出风格规范：[specs/cli-output-style.md](specs/cli-output-style.md)
2. 控制台颜色参考：[specs/console-color.md](specs/console-color.md)
3. 退出码策略：`apps/tracer_windows_cli/src/api/cli/EXIT_CODE_POLICY.md`

## 历史遗留入口
1. `docs/time_tracer/guides/cli/command_index.md`

## 规则
1. 新增 Windows CLI 文档优先放在本目录。
2. 历史 CLI 文档保留，不强制搬迁。
3. Windows CLI 通过 `tracer_core + tracer_adapters` 完成装配，不直接依赖旧路径 `apps/time_tracer/src/infrastructure/io/**`。

## Core ABI 约定
1. Windows CLI 通过动态加载 `time_tracer_core.dll` 调用 Core C ABI。
2. 运行时符号前缀统一为 `tracer_core_*`。
3. 具体契约见：`docs/time_tracer/core/contracts/c_abi.md`。
4. DataQuery 统计契约见：`docs/time_tracer/core/contracts/stats/capability_matrix_v1.md`。
5. semantic_json 版本策略见：`docs/time_tracer/core/contracts/stats/semantic_json_versioning_policy.md`。
6. Adapter 边界清单见：`docs/time_tracer/core/contracts/stats/adapter_reviewer_checklist.md`。

## Report 表现层定位（与 Android 区分）
1. Windows CLI 在 Report 场景的定位是 `文字分析 + 格式化报告`（面向终端输出与文本流水线）。
2. 当前不提供图形化折线图渲染能力。
3. Android 在同一 core 数据能力之上提供 `文字 / 图表` 双模式，属于 Android UI 专属表现层能力。
4. Android 侧细节见：`docs/time_tracer/clients/android_ui/features.md`。
