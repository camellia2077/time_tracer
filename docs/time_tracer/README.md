# TimeTracer 文档入口

本文件仅做分层索引，不承载实现细节。

## 演进历程 (Evolution Path)
1. **起源时代 (The CLI Era)**：最初仅作为桌面端 CLI 工具，用户在手机上记录 TXT 日志，后需手动通过蓝牙传输至电脑，再通过 CLI 完成入库与分析。
2. **"Apollo-Soyuz" 对接时代**：从 2026 年 2 月 14 日开始，截止到北京时间 2026 年 2 月 23 日 00:21，项目在春节期间成功实现了 Core 引擎逻辑（`time_tracer_core`）与 IO 层、表现层的解耦。
3. **多端融合 (Platform Convergence)**：Android UI 表现层上线，通过 JNI 实现 100% 核心代码复用。移动端从此具备原生解析能力，打破了平台间的传输壁垒。

## 分层原则
1. **契约层 (What)**：稳定对外协议，放在 `docs/time_tracer/core/` 与 `docs/time_tracer/clients/android_ui/`。
2. **实现层 (How)**：底层解析机制、代码算法等，放在 `docs/time_tracer/guides/` 等研发域。
3. **展现与使用层 (User)**：针对终端用户的交互操作、排版规范以及日志书写技巧，放在 `docs/time_tracer/user_manual/` 及对应 Client 目录。

## 变更落点
1. 修改 `tracer_core_*` C ABI、JSON 字段契约、响应包结构：更新 `docs/time_tracer/core/contracts/c_abi.md`。
2. 修改 JNI <-> Kotlin 协议、Android runtime 网关行为：更新 `docs/time_tracer/clients/android_ui/runtime-protocol.md`。
3. 修改 envelope/fields/runtime codec 实现：更新 `modules/tracer_transport/README.md`（并按需在契约层文档补充结论）。
4. 修改 Android/CLI 的表现层差异（例如 Android 新增图表、CLI 仅文本）：同时更新
   - `docs/time_tracer/clients/android_ui/features.md`
   - `docs/time_tracer/clients/windows_cli/README.md`

## 表现层能力差异（Android vs Windows CLI）
1. 该类差异属于“展现层（UI/CLI）”范围，不属于 Core ABI 契约差异。
2. Android Report 目前支持 `文字 / 图表` 双模式（图表为 Compose 折线图）。
3. Windows CLI 目前定位为 `文字分析 + 格式化报告`，不提供图形化折线图渲染。
4. 详细说明见：
   - Android：`docs/time_tracer/clients/android_ui/features.md`
   - Windows CLI：`docs/time_tracer/clients/windows_cli/README.md`

## 核心入口
1. **开发者指南域**：`docs/time_tracer/guides/` (例如：[数据结构与算法](docs/time_tracer/guides/database/README.md))
2. **终端用户手册域 (强烈推荐使用者阅读)**：`docs/time_tracer/user_manual/` (包含输入文本规范 `input_format_cn.md` 等)
3. Core 文档域：`docs/time_tracer/core/README.md`
4. C ABI 契约：`docs/time_tracer/core/contracts/c_abi.md`
5. Android Runtime 协议：`docs/time_tracer/clients/android_ui/runtime-protocol.md`
6. Windows CLI 文档：`docs/time_tracer/clients/windows_cli/README.md`
7. Transport 实现说明：`modules/tracer_transport/README.md`
8. 跨层流程：`docs/time_tracer/workflows/workflow.md`
9. DataQuery 职责边界与代码落点：`docs/time_tracer/core/architecture/data_query/data_query_responsibility_boundaries_v1.md`
