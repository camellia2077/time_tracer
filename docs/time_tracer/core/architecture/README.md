# Core Architecture

本目录放置 Core 架构与边界约束文档。

## 关注问题
1. 多端复用：同一套 Core 统计与查询能力需要被 Windows CLI 与 Android 共同复用，避免端侧重复实现与口径漂移。
2. 数据与渲染解耦：统计计算、时间范围编排、输出渲染需要分层，避免“业务逻辑混入展示层”。
3. 入口适配瘦身：Core/CLI/Android 入口层只做参数映射、调用转发、结果解码与错误边界。

## 架构原则
1. 统计公式只在 `stats/` 维护。
2. 时间范围与 action 编排只在 `orchestrators/` 维护。
3. `text`/`semantic_json` 输出只在 `renderers/` 维护。
4. 适配层不承载统计与编排实现，只承载 IO 与表现层映射。

## 推荐阅读顺序
1. 先看边界规则：明确“什么代码应该放在哪一层”。
2. 再看职责落点：按文件路径快速定位实现。
3. 最后看收口清单：确认迁移完成状态与后续维护基线。

## 文档
1. `docs/time_tracer/core/architecture/data_lifecycle_parsing_to_storage.md`
   - **必读**：Time Tracer 简明数据流转生命周期大图（从 Parse -> Domain -> Importer -> SQLite）。
2. `docs/time_tracer/core/architecture/domain_model_and_rules.md`
   - Core Domain 层架构：时间差运算算法、通宵动态衔接逻辑、防断与安全验证规则。
3. `docs/time_tracer/core/architecture/application_pipeline_and_ports.md`
   - Application 层架构：流水线编排驱动、前后解析器降维处理（TextParser & MemoryParser）、Ports 防腐层映射。
4. `docs/time_tracer/core/architecture/infrastructure_persistence.md`
   - Infrastructure 持久层架构：SQLite 高速无脑入库（Writer）、动态生成层级项目树（Project Resolver）、基于关联型数据库构建底层聚合查询（DataQueryService）与依赖隔离。
5. `docs/time_tracer/core/architecture/refactor_module_boundaries.md`
   - Core 分层与模块边界重构与开发规则说明。
2. `docs/time_tracer/core/architecture/data_query/README.md`
   - DataQuery 子域总览与阅读顺序。
3. `docs/time_tracer/core/architecture/data_query/data_query_refactor_completion_v1.md`
   - DataQuery 重构收口结果与稳定化约束。
4. `docs/time_tracer/core/architecture/data_query/data_query_responsibility_boundaries_v1.md`
   - 按职责边界拆分后的代码落点（含 Core/CLI/Android 路径）。
