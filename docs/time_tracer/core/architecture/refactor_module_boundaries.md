# 模块拆分职责图（Refactor 2026-02）

本文记录近期重构后，核心长文件拆分结果的职责边界、入口函数与依赖关系。

## 1. Data Query（SQLite）

### 入口层
- 文件：`apps/tracer_core/src/infrastructure/query/data/data_query_repository.cpp`
- 对外入口：
  - `QueryYears`
  - `QueryMonths`
  - `QueryDays`
  - `QueryDatesByFilters`
  - `QueryDayDurations`
  - `QueryProjectRootNames`
  - `QueryDayDurationsByRootInDateRange`
  - `QueryActivitySuggestions`
  - `QueryLatestTrackedDate`
  - `QueryProjectTree`
- 责任：仅保留请求编排与结果汇总，不再承载大段 SQL 拼装和手写 row decode。

### SQL Builders 层
- 文件：`apps/tracer_core/src/infrastructure/query/data/data_query_sql_builders.cpp`
- 责任：
  - SQL 片段构建与条件拼装。
  - `project_path_snapshot` 列存在性检查。
  - lookback/limit 的输入归一化辅助。

### Row Mappers / Executors 层
- 文件：`apps/tracer_core/src/infrastructure/query/data/data_query_row_mappers.cpp`
- 责任：
  - sqlite 参数绑定。
  - `ActivitySuggestionRow` / project tree records 的取行解码。
  - 统一 prepare/step/finalize 执行路径。

### 共享契约
- 文件：`apps/tracer_core/src/infrastructure/query/data/data_query_repository_internal.hpp`
- 责任：跨 TU 的最小内部接口，不暴露到应用层。

### 依赖方向
1. `data_query_repository.cpp` -> `data_query_sql_builders.cpp`
2. `data_query_repository.cpp` -> `data_query_row_mappers.cpp`
3. `data_query_sql_builders.cpp` / `data_query_row_mappers.cpp` -> `data_query_repository_sql.cpp`
4. 全部保持在 infrastructure 内部，不反向依赖 application/domain。

## 2. Android JNI Bridge

### Shared Helpers / State
- 文件：`apps/tracer_core/src/api/android/native_bridge.cpp`
- 责任：
  - `g_runtime_mutex` / `g_runtime` 生命周期状态。
  - 字符串编解码、响应封装、枚举解析、core 响应解析。

### Native Calls
- 文件：`apps/tracer_core/src/api/android/native_bridge_calls.cpp`
- 入口：
  - `NativeInit`
  - `NativeIngest`
  - `NativeValidateStructure`
  - `NativeValidateLogic`
  - `NativeQuery`
  - `NativeReport`
- 责任：JNI 参数读取 -> runtime request 构造 -> core C API 调用 -> envelope 返回。

### Registration
- 文件：`apps/tracer_core/src/api/android/native_bridge_registration.cpp`
- 入口：
  - `JNI_OnLoad`
  - `JNI_OnUnload`
  - `TryRegisterNativeMethods`
- 责任：native method 注册和桥接加载/卸载。

### 共享契约
- 文件：`apps/tracer_core/src/api/android/native_bridge_internal.hpp`
- 责任：桥接模块内部最小接口（含模板化异常包装）。

### 依赖方向
1. `native_bridge_registration.cpp` -> `native_bridge_calls.cpp`
2. `native_bridge_calls.cpp` -> `native_bridge.cpp`
3. 三者统一经 `native_bridge_internal.hpp` 共享契约。

## 3. Converter Core

### 入口编排层
- 文件：`apps/tracer_core/src/domain/logic/converter/convert/core/converter_core.cpp`
- 入口：
  - `DayProcessor::Process`
  - `LogLinker::LinkLogs`
- 责任：流程编排与跨日逻辑，不再内联复杂映射/统计细节。

### Stats 层
- 文件：`apps/tracer_core/src/domain/logic/converter/convert/core/converter_core_stats.cpp`
- 责任：
  - `DayStats::CalculateStats`
  - 时间标准化与 duration/时间戳计算辅助
  - source span 合并

### Activity Mapping 层
- 文件：`apps/tracer_core/src/domain/logic/converter/convert/core/converter_core_activity_mapper.cpp`
- 责任：
  - `ActivityMapper::MapActivities`
  - 文本映射、时长规则映射、顶级父节点映射、活动追加。

### 共享契约
- 文件：`apps/tracer_core/src/domain/logic/converter/convert/core/converter_core_internal.hpp`
- 责任：域内拆分后的内部类型和函数声明。

### 依赖方向
1. `converter_core.cpp` -> `converter_core_activity_mapper.cpp`
2. `converter_core.cpp` -> `converter_core_stats.cpp`
3. 两子模块共享 `converter_core_internal.hpp`，不引入 infrastructure 依赖。

## 4. 测试文件拆分

`time_tracker_android_runtime_tests` 场景拆分为：
1. `apps/tracer_core/src/infrastructure/tests/android_runtime_smoke_tests.cpp`
2. `apps/tracer_core/src/infrastructure/tests/android_runtime_core_config_tests.cpp`
3. `apps/tracer_core/src/infrastructure/tests/android_runtime_business_regression_tests.cpp`

目的：将 smoke/config/regression 分开，失败定位更直接。
