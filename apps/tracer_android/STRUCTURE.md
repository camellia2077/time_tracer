# tracer_android 目录说明

本文说明 `apps/tracer_android` 的当前分层结构（Feature + Contract + Runtime）与职责边界。

## 1. 模块总览

- `:app`
  - 应用壳层与组装层（Composition Root）。
  - 通过 `TracerApplication + AppContainer` 创建并注入 `RuntimeGateway`。
- `:contract`
  - 跨模块稳定契约层。
  - 定义 `RuntimeGateway` 接口与对外 DTO（call/result models）。
- `:feature-data`
  - Data 页面 UI + ViewModel（初始化、导入、清理、debug 操作）。
- `:feature-report`
  - Report 页面 UI + ViewModel（Day/Month/Week/Year/Recent/Range）。
- `:feature-record`
  - Record 与 TXT 编辑页面 UI + ViewModel/UseCases。
- `:runtime`
  - Runtime 实现层（JNI 调用、资产复制、TXT/TOML 存储、同步用例）。
  - 实现 `contract.RuntimeGateway`。

## 2. 依赖方向

- `app -> contract + runtime + feature-*`
- `feature-* -> contract`
- `runtime -> contract`

说明：

- feature 模块不直接依赖 runtime 实现细节。
- app 是唯一知道 `NativeRuntimeController` 具体实现的位置。

## 3. 目录结构（核心）

```text
apps/tracer_android/
  app/
    src/main/java/com/example/tracer/
      MainActivity.kt
      TracerApplication.kt
      di/AppContainer.kt
      ui/screen/TracerScreen.kt
      ui/screen/ConfigScreen.kt
      ui/viewmodel/ConfigViewModel.kt

  contract/
    src/main/java/com/example/tracer/
      RuntimeGateway.kt
      RuntimeGatewayModels.kt

  feature-data/
    src/main/java/com/example/tracer/ui/screen/DataScreen.kt
    src/main/java/com/example/tracer/ui/viewmodel/DataViewModel.kt

  feature-report/
    src/main/java/com/example/tracer/ui/screen/QueryReportScreen.kt
    src/main/java/com/example/tracer/ui/viewmodel/QueryReportViewModel.kt

  feature-record/
    src/main/java/com/example/tracer/ui/screen/RecordScreen.kt
    src/main/java/com/example/tracer/ui/screen/TxtEditorScreen.kt
    src/main/java/com/example/tracer/ui/viewmodel/RecordViewModel.kt
    src/main/java/com/example/tracer/ui/viewmodel/RecordUseCases.kt

  runtime/
    src/main/cpp/CMakeLists.txt
    src/main/java/com/example/tracer/bridge/NativeBridge.kt
    src/main/java/com/example/tracer/runtime/*
    src/main/assets/time_tracer/*
```

## 4. 各层职责

## app

- `TracerApplication`：创建并持有 `AppContainer`。
- `AppContainer`：创建 `RuntimeGateway` 实例。
- `MainActivity`：从容器取 `RuntimeGateway` 并注入 UI。
- `TracerScreen`：顶层 Tab 拼装。

## contract

- 暴露稳定 API：
  - `RuntimeGateway`
  - `NativeCallResult`、`ReportCallResult`、`RecordActionResult` 等 DTO

## feature-*

- 只依赖 contract，不触碰 JNI/文件系统细节。
- 负责：
  - 输入校验
  - 状态管理
  - 交互编排

## runtime

- 负责：
  - Native 初始化与桥接调用
  - Runtime 路径准备
  - live txt/config toml 存储
  - ingest 同步用例
- 对外只通过 `RuntimeGateway` 返回 contract DTO。
- 内部实现模型保持 `internal`。

## 5. 运行时数据位置

资产目录（来自 runtime module）：

- `runtime/src/main/assets/time_tracer/**`

启动后复制到：

- `<filesDir>/time_tracer/**`

常用路径：

- DB：`<filesDir>/time_tracer/db/time_data.sqlite3`
- Config：`<filesDir>/time_tracer/config/**`
- Input：`<filesDir>/time_tracer/input/**`
- Output：`<filesDir>/time_tracer/output/**`

## 6. 回归入口

- 回归清单：`apps/tracer_android/REGRESSION_CHECKLIST.md`
- 架构分阶段任务：`temp/task.md`

