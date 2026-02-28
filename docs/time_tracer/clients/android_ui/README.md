# Android UI 文档域

本目录用于承载 Android UI 展现层相关文档。

## 范围
1. 页面结构、模块分层、交互状态。
2. Compose 层与 Runtime Gateway 的边界说明。
3. Android 打包与发布流程说明（UI 视角）。

## 对应代码
1. `apps/tracer_android/`
2. `apps/tracer_android/app/`
3. `apps/tracer_android/feature-*`
4. `apps/tracer_android/runtime/`

## 原则与规范
1. Agent 快速上手入口：[specs/AGENT_ONBOARDING.md](specs/AGENT_ONBOARDING.md)
2. 架构设计：[architecture.md](architecture.md)
3. Android Runtime 协议：[runtime-protocol.md](runtime-protocol.md)
4. 偏好存储设计：[specs/preference-storage.md](specs/preference-storage.md)
5. 多语言按钮同步：[specs/i18n-button-sync.md](specs/i18n-button-sync.md)
6. DataQuery 统计契约：`docs/time_tracer/core/contracts/stats/capability_matrix_v1.md`
7. semantic_json 版本策略：`docs/time_tracer/core/contracts/stats/semantic_json_versioning_policy.md`
8. Adapter 边界清单：`docs/time_tracer/core/contracts/stats/adapter_reviewer_checklist.md`

## 指南与历史
1. APK 编译指南：[apk-compilation-guide.md](apk-compilation-guide.md)
2. 运行历史里程碑：[HISTORY.md](HISTORY.md)
3. 应用更新日志：[CHANGELOG.md](CHANGELOG.md)
4. 归档历史记录：[history/README.md](history/README.md)

## 规则
1. 新增 Android UI 文档优先放在本目录。
2. 历史 Android 文档保留，不强制搬迁。
3. Android runtime 通过 `tracer_core + tracer_adapters` 复用同一套 adapter 组装链路（含 IO adapter），不直接绑定旧目录实现。
4. Android 业务数据协议文档统一放在 `runtime-protocol.md`；C ABI 全局规则以 `docs/time_tracer/core/contracts/c_abi.md` 为准。

## Tree 表现层约定（Phase 4/5）
1. Android Report 的 Tree 查询优先走结构化链路：`nativeTree -> runtime_tree_json`。
2. UI 层基于结构化节点渲染（层级缩进、展开/折叠），不再以“解析文本树”驱动业务状态。
3. 兼容期保留 `query data tree` 文本 fallback（仅兜底），避免升级窗口内回归。
