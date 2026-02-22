# Adapter Code Map (Stats/DataQuery)

## Core Adapter（入口适配）
1. `apps/time_tracer/src/infrastructure/persistence/sqlite_data_query_service.cpp`
   - 请求入口、action 分发起点、错误边界。
2. `apps/time_tracer/src/infrastructure/persistence/sqlite_data_query_service_dispatch.cpp`
   - 轻量路由（调用 orchestrators）。
3. `apps/time_tracer/src/infrastructure/persistence/sqlite_data_query_service_report_mapping.cpp`
   - `report-chart` payload 组装，调用 `orchestrators + stats`。

## Windows CLI Adapter
1. `apps/tracer_windows_cli/src/api/cli/impl/commands/query/data_query_parser.cpp`
   - 参数解析与 request 归一化。
2. `apps/tracer_windows_cli/src/api/cli/impl/commands/query/query_command.cpp`
   - 请求提交与输出展示。
3. `apps/tracer_windows_cli/src/bootstrap/cli_runtime_factory_proxy.cpp`
   - transport/runtime 请求映射与响应解码。

## Android Adapter
1. `apps/tracer_android/runtime/src/main/java/com/example/tracer/runtime/controller/RuntimeQueryDelegate.kt`
   - Query 请求组织与 runtime 调用。
2. `apps/tracer_android/runtime/src/main/java/com/example/tracer/runtime/NativeRuntimeQueryOps.kt`
   - payload 解码、fallback 标记、ViewModel 字段映射。
3. `apps/tracer_android/runtime/src/main/java/com/example/tracer/runtime/NativeRuntimeController.kt`
   - runtime 总入口与生命周期管理。

## 自动化守卫（防回流）
1. `apps/time_tracer/src/infrastructure/tests/data_query_refactor_tests.cpp`
   - `TestAdapterBoundaryGuardrails` 扫描 adapter 文件中的禁止 token：
   - CLI/Android adapter 禁止出现统计公式与 core 编排函数调用。
   - Core adapter 禁止出现手写统计公式实现片段。
