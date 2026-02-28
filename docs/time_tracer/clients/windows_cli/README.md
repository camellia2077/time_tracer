# Windows CLI 文档域

本目录用于承载 Windows CLI 展现层相关文档。

## 范围
1. 命令行命令与参数语义。
2. CLI 输出格式、交互规范、退出码策略。
3. Windows 交付与打包相关说明。

## 对应代码
1. `apps/tracer_cli/windows/`
2. `apps/tracer_cli/windows/src/api/cli/`

## 测试套件约定
1. `tracer_windows_cli` 套件是 `core + windows cli` 集成套件。
2. 推荐验证命令：`python scripts/run.py verify --app tracer_core --quick`。
3. 结果输出目录：`test/output/tracer_windows_cli/`（含 `result.json`、`result_cases.json`、`logs/`）。
4. runtime guard 复用入口：`python test/runtime_guard.py --build-dir build_fast`（或 `test/run_runtime_guard.bat --build-dir build_fast`）。
5. 冒烟统一入口：`python test/run_smoke_tracer_windows_cli.py --build-dir build_fast`。

## Phase 5 验收记录（2026-02-22）
1. `python scripts/run.py verify --app tracer_core --quick` 已通过。
2. `release_safe` 体积对比完成：`time_tracer_cli.exe` 与关键段 `.text/.rdata` 相对基线无增量（`delta=0`）。
3. 手工冒烟已覆盖：`--version`、`--help`、缺 core dll、缺 config、典型 `query/export`。
4. 异常场景统一由 `Runtime check failed: ...` 报错，便于定位缺失运行时文件。

## 原则与规范
1. 结构边界说明：[specs/STRUCTURE.md](specs/STRUCTURE.md)
2. 输出风格规范：[specs/cli-output-style.md](specs/cli-output-style.md)
3. 控制台颜色参考：[specs/console-color.md](specs/console-color.md)
4. 退出码策略：`apps/tracer_cli/windows/src/api/cli/EXIT_CODE_POLICY.md`

## 历史遗留入口
1. `docs/time_tracer/guides/cli/command_index.md`

## 规则
1. 新增 Windows CLI 文档优先放在本目录。
2. 历史 CLI 文档保留，不强制搬迁。
3. Windows CLI 通过 `tracer_core + tracer_adapters` 完成装配，不直接依赖旧路径 `apps/tracer_core/src/infrastructure/io/**`。

## Core ABI 约定
1. Windows CLI 通过动态加载 `time_tracer_core.dll` 调用 Core C ABI。
2. 运行时符号前缀统一为 `tracer_core_*`。
3. 具体契约见：`docs/time_tracer/core/contracts/c_abi.md`。
4. DataQuery 统计契约见：`docs/time_tracer/core/contracts/stats/capability_matrix_v1.md`。
5. semantic_json 版本策略见：`docs/time_tracer/core/contracts/stats/semantic_json_versioning_policy.md`。
6. Adapter 边界清单见：`docs/time_tracer/core/contracts/stats/adapter_reviewer_checklist.md`。

## 体积基线（release_safe）
1. 工具链入口：`python scripts/run.py artifact-size --app tracer_windows_cli --targets time_tracer_cli --profile release_safe`
2. 默认统计目标：`time_tracer_cli.exe`
3. JSON 输出包含总大小与 `.text/.rdata` 段信息（用于版本对比）。

## MinGW 工具链说明（`std::print` 与 `stdc++exp`）
1. 截至 `2026-02-24`，Windows + MinGW 下 `std::print/std::println` 仍依赖 `-lstdc++exp`。
2. 结论、最小复现与后续复查条件见：
   - `docs/time_tracer/guides/build_guide.md` 的 `MinGW std::print / stdc++exp 兼容性结论（2026-02-24）`。

## Report 表现层定位（与 Android 区分）
1. Windows CLI 在 Report 场景的定位是 `文字分析 + 格式化报告`（面向终端输出与文本流水线）。
2. Windows CLI 提供 `report-chart` 单文件 HTML 图表导出：
   - 命令：`chart ... -o <path>.html [--type line|bar|pie|heatmap-year|heatmap-month] [--theme default|github]`
   - 默认图表类型：`line`
   - 默认主题：`default`（`github` 主题当前用于热力图配色）
   - 数据源：同一份 core `report-chart` payload（不新增业务统计口径）。
3. 当前 Windows 已支持 `Line / Bar / Pie / Heatmap-Year / Heatmap-Month` 五种图表类型导出（同一命令入口）。
4. Android 在同一 core 数据能力之上提供 `文字 / 图表` 双模式，属于 Android UI 专属表现层能力。
5. Android 侧细节见：`docs/time_tracer/clients/android_ui/features.md`。
6. HTML 导出使用 vendored 本地 `ECharts` 脚本并内嵌，不依赖运行时 CDN（离线可用）。

## Tree 分层约定（Phase 3/5）
1. Core 负责提供结构化 Tree 数据能力（`RunTreeQuery` / `runtime_tree_json`）。
2. Windows CLI 只负责参数解析与终端渲染，不承担 tree 业务推断逻辑。
3. `tree` 命令参数模型：
   - `<root>`：位置参数（可选）
   - `-l, --level`：最大层级
   - `-r, --roots`：列出根节点
4. CLI 已移除 `time_tracker/tree/query` 等手工字符串过滤分支，统一依赖 `CommandValidator + GetPositionalArgs(definitions)`。

## Crypto 进度显示（导入/导出长耗时反馈）
1. `crypto encrypt/decrypt` 已接入进度回调并在 CLI 展示：
   - 总体进度：`overall`（跨年份/全部 txt 的总字节或总文件）
   - 分组进度：`group`（如 `2026 (3/12)`）
   - 当前文件进度：`file`（当前 txt 百分比与字节）
2. 命令示例：
```bash
time_tracer_cli crypto encrypt --in "C:/data/txt" --out "C:/data/encrypted"
time_tracer_cli crypto decrypt --in "C:/data/encrypted" --out "C:/data/txt_out"
```
3. 输出示例（TTY 单行刷新模式）：
```text
[crypto] encrypt | phase: read | status: running | overall: 42% (84.0MB/200MB) | group: 2026 (3/12) | file: 2026-03.txt 66% (2.64MB/4.00MB)
```
4. 输出示例（非 TTY / 不支持 ANSI 降级模式）：
```text
[crypto] decrypt | phase: scan | status: running | overall: 0% (0B/155B)
[crypto] decrypt | phase: derive-key | status: running | overall: 100% (155B/155B) | group: (root) (1/1) | file: 2025-01.tracer 100% (155B/155B)
[crypto] decrypt | phase: failed | status: failed | overall: 100% (155B/155B) | group: (root) (1/1) | file: 2025-01.tracer 100% (155B/155B)
```
5. 终态约定：
   - 成功：`status: success`
   - 失败：`status: failed`
   - 取消：`status: cancelled`

