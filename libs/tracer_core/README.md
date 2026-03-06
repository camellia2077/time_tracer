# tracer_core_lib

`tracer_core_lib` 是 `tracer_core` 迁移到 `libs/` 的复用聚合入口。

已完成项：

- Step 1.1：建立独立 CMake target（`tracer_core_lib`）。
- Step 1.2（最小单元）：迁移 `shared` 低耦合首批单元到 `libs/tracer_core`：
  - `shared/utils/period_utils.cpp`
  - `shared/modules/tracer.core.shared*`
- Step 2.1：迁移 `domain` 构建职责到 `tracer_core_domain_lib`。
- Step 2.2：迁移 `application` 构建职责到 `tracer_core_application_lib`。
- Step 2.3：迁移低耦合 `infrastructure` 到
  `tracer_core_infrastructure_lite_lib`（logging/clock/static config + modules）。
- Step 3.2：迁移剩余 `infrastructure` 构建职责到
  `libs/tracer_core/infrastructure/CMakeLists.txt`（保留 `reports_shared`、
  `reports_data`、`ttri`、`tti`、`time_tracker_infrastructure`、
  `tracer_adapters` 兼容目标名）。

当前仍保持头文件接口兼容与业务行为不变。
