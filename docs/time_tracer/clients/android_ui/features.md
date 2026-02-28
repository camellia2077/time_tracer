# Android UI 功能特性说明

本文档记录 Android 端 UI 的交互特性与功能细节。

## Record Activity

### 插入规则：末尾时间守卫（Append-Only）
- 目标：`Record Activity` 只用于“记录当前活动”，不用于补录历史时间点。
- 判定规则：只有 `newTime > 当天最后一条活动时间` 才允许写入。
- 时间比较：按 `HHmm -> 分钟值 (HH * 60 + mm)` 转换后比较，不使用直接减法结果做业务判断。
- 拒绝策略：若 `newTime <= lastTime`，直接拒绝插入并返回明确错误，引导用户使用 `DAY/ALL` 编辑器做历史补录。

### 设计思路
1. 保证自然时间线单调递增，避免 `Record Activity` 在日块中间插入导致时序混乱。
2. 将“实时记录”和“历史修订”职责分离：实时记录走 `Record Activity`，历史修订走 TXT 编辑窗口。
3. 与 `DAY` 的“日块滑动上下文窗口算法”协同：DAY 负责局部改写 ALL，Record Activity 负责追加。

### 实现位置（Kotlin）
- `apps/tracer_android/runtime/src/main/java/com/example/tracer/runtime/LiveRawRecordStore.kt`
  - `RecordParsing.parseHhmmToMinutes`：`HHmm` 严格解析与分钟值转换。
  - `RecordParsing.findLastValidEventTimeToken`：提取当日日块最后一条合法时间。
  - `RecordPersistence.insertEventIntoDayBlock`：执行 `newTime > lastTime` 校验，失败时抛出明确错误；成功时仅在日块尾部追加。
- `apps/tracer_android/runtime/src/main/java/com/example/tracer/runtime/controller/RuntimeRecordDelegate.kt`
  - `recordNow` 链路接收上述异常并将错误消息回传 UI。

## Data：TXT 存储与单 TXT 导入

### 目标
- Android 端将 TXT 作为“受管输入数据源”统一管理，而不是依赖外部文件名约定。
- 支持在 Data 页面导入单个 TXT，并按文件内容头解析所属月份。
- 导入后立即走 core ingest，保证“文件状态”和“数据库状态”同步。

### 存储模型（Android 受管目录）
- 运行时根目录：`<app_files>/time_tracer`
- 月度 TXT 主存储目录：`input/full`
- 月文件命名：`YYYY-MM.txt`
- 设计原则：
  - 外部导入文件先进入应用临时区（cache staging）。
  - 通过内容解析确定目标月后，写入受管目录。
  - 后续查询、编辑、入库均基于受管目录内的月文件。

### 单 TXT 导入规则
- 入口：Data 页面 `Import Single TXT`。
- 月份判定：只认文件内容头 `yYYYY + mMM`。
- 文件名不参与月份判定（可忽略用户文件名中的日期信息）。
- 写入策略：
  - 受管目录不存在该月文件：直接写入新文件。
  - 已存在该月文件：覆盖该月文件（同月替换）。
- 入库策略：写入/覆盖后调用 core `single_txt_replace_month` ingest 模式。

### 导入处理链路（Android）
1. 用户在 Data 页选择单个文档。
2. 选中文件拷贝到 app cache 暂存目录。
3. Runtime 读取文本并做标准化（BOM/换行）。
4. Runtime 从内容中解析 `yYYYY + mMM`，生成目标月键 `YYYY-MM`。
5. 写入/覆盖 `input/full/YYYY-MM.txt`。
6. 调 JNI -> core ingest（`single_txt_replace_month`）完成数据库更新。

### 代码定位（Kotlin/Native）
- `apps/tracer_android/feature-data/src/main/java/com/example/tracer/ui/screen/DataScreen.kt`
  - Data 页面导入按钮与回调入口。
- `apps/tracer_android/app/src/main/java/com/example/tracer/ui/screen/tracer/TracerScreenImports.kt`
  - `OpenDocument` 选择、导入文件 staging。
- `apps/tracer_android/feature-data/src/main/java/com/example/tracer/ui/viewmodel/DataViewModel.kt`
  - `ingestSingleTxtReplaceMonth` 导入动作编排与状态回传。
- `apps/tracer_android/runtime/src/main/java/com/example/tracer/runtime/NativeRuntimeController.kt`
  - 头解析（`yYYYY + mMM`）、受管路径写入、触发 native ingest。
- `apps/tracer_android/runtime/src/main/java/com/example/tracer/bridge/NativeBridge.kt`
  - `nativeIngestSingleTxtReplaceMonth` JNI 声明。
- `apps/tracer_core/src/api/android/native_bridge_calls.cpp`
  - 组装 ingest 请求并设置 `ingest_mode=single_txt_replace_month`。

## Config：Android TOML 导入（Full Replace / Partial Update）

### 目标
- 支持在 `Config` 页面导入 Android 配置 TOML 目录，并替换本地配置。
- 导入严格复用导出白名单规则，避免“导出一套/导入一套”。

### 固定路径白名单（命名不可变）
- `config.toml`
- `meta/bundle.toml`
- `converter/interval_processor_config.toml`
- `converter/alias_mapping.toml`
- `converter/duration_rules.toml`
- `reports/markdown/day.toml`
- `reports/markdown/month.toml`
- `reports/markdown/period.toml`
- `reports/markdown/week.toml`
- `reports/markdown/year.toml`

### 导入模式
- `Full Replace`：
  - 要求 10 个固定路径文件全量齐全。
  - 路径必须直接命中白名单（`relativePath` 严格匹配）。
  - 任一必需文件缺失/路径非法/语法错误，整体失败。
- `Partial Update`：
  - 只处理“文件名命中白名单”的 TOML（支持从任意子目录识别并映射到固定目标路径）。
  - 未命中白名单的 TOML 直接跳过，不更新。
  - 仅对识别到的文件做语法校验与更新；若包含 `meta/bundle.toml`，则必须 `profile = "android"`。
  - 同一目标路径若出现重复来源，整体失败（避免不确定覆盖）。

### 替换策略
- `Full Replace` 与 `Partial Update` 均采用“先校验后替换”。
- 替换前先读取旧内容做内存备份；替换过程中若任一文件失败，按已替换顺序逆序回滚，避免半替换状态。
- 成功后刷新 Config 文件列表与编辑器内容。

### 初始化覆盖策略
- `RuntimeEnvironment` 初始化时，`config` 目录改为“仅缺失补齐”，不覆盖已存在 TOML。
- 导入后的用户配置在后续 runtime 初始化时默认保持不变。

### 代码定位（Kotlin）
- `apps/tracer_android/app/src/main/java/com/example/tracer/ui/viewmodel/AndroidConfigPathPolicy.kt`
  - 导入/导出共用白名单与路径归一化规则。
- `apps/tracer_android/app/src/main/java/com/example/tracer/ui/viewmodel/ConfigViewModel.kt`
  - Config 页状态管理与导入导出入口编排（委托 use case 执行）。
- `apps/tracer_android/app/src/main/java/com/example/tracer/ui/viewmodel/ConfigBundleTransferUseCase.kt`
  - 导入前校验、`profile` 校验、替换与回滚。
- `apps/tracer_android/app/src/main/java/com/example/tracer/ui/viewmodel/ConfigBundleTransferModels.kt`
  - Config 导入导出数据模型（export/import entry 与结果对象）。
- `apps/tracer_android/app/src/main/java/com/example/tracer/ui/screen/tracer/TracerScreenImports.kt`
  - `OpenDocumentTree` 导入入口与目录读取触发。
- `apps/tracer_android/app/src/main/java/com/example/tracer/ui/screen/TracerScreenDocumentTree.kt`
  - 目录递归读取 TOML 文本。
- `apps/tracer_android/runtime/src/main/java/com/example/tracer/runtime/RuntimeEnvironment.kt`
  - 初始化阶段配置文件“仅补齐不覆盖”策略。

## TXT 编辑窗口：ALL 与 DAY

### 设计目标
- `ALL` 用于查看/编辑整个月 TXT。
- `DAY` 使用“日块滑动上下文窗口算法”作为 `ALL` 的局部窗口编辑模式，用于在长文本中快速定位并编辑某一天。

### 日块滑动上下文窗口算法
- 锚点：`Target Day (MMDD)`。
- 分块规则：在月 TXT 中定位目标 `MMDD` 行，窗口范围为：
  `目标 MMDD 行下一行` 到 `下一个 MMDD 行之前`。
- 编辑框内容：仅显示窗口正文（备注行、`HHmm + 活动名` 行），不显示 `MMDD` 头行。
- 保存行为：仅替换该窗口正文，保留原始 `MMDD` 头行与其他日期块内容不变。
- 边界约束：DAY 模式只编辑“已存在的日期块”；若目标 `MMDD` 不存在，不创建新区块。

### Save & Sync 流程（DAY/ALL 通用）
1. 先保存 TXT 到源文件。
2. 运行结构校验（C++ pipeline）。
3. 运行逻辑校验（C++ pipeline）。
4. 校验通过后再执行 ingest 入库。

说明：该流程复用 core 校验，不单独维护一套 DAY 规则。

### 草稿与提交策略（TXT/Config 通用）
- 编辑框内容默认视为“草稿”。
- 只有点击保存（TXT 的 `Save & Sync` / Config 的 `Save Changes`）才会落盘。
- 未保存草稿在以下场景会被直接丢弃（不弹窗确认）：
  - 切换到其他底部标签页。
  - 应用进入后台/退出（`ON_STOP`）。
- 丢弃后再次进入编辑页，显示的是“最后一次已保存”的内容。

### 代码定位（Kotlin）
- `apps/tracer_android/feature-record/src/main/java/com/example/tracer/ui/screen/TxtEditorScreen.kt`
  - 日块窗口提取（`buildDayBlockEditorState`）、窗口替换（`mergeDayBlockContent`）的核心实现。
- `apps/tracer_android/feature-record/src/main/java/com/example/tracer/ui/screen/TxtEditorContentCard.kt`
  - `ALL/DAY` 切换、`DAY Content` 文本框行为、`Save & Sync` 按钮可用条件。
- `apps/tracer_android/feature-record/src/main/java/com/example/tracer/ui/viewmodel/RecordUseCases.kt`
  - 编辑内容保存入口（`saveHistoryFileAndSync`）与 UI 状态更新。
- `apps/tracer_android/runtime/src/main/java/com/example/tracer/runtime/controller/RuntimeRecordDelegate.kt`
  - 运行时保存流程：写文件 -> validate structure -> validate logic -> ingest。
- `apps/tracer_android/runtime/src/main/java/com/example/tracer/bridge/NativeBridge.kt`
  - Native 方法声明（`nativeValidateStructure` / `nativeValidateLogic` / `nativeIngest`）。
- `apps/tracer_android/feature-record/src/main/java/com/example/tracer/ui/viewmodel/RecordViewModel.kt`
  - `discardUnsavedHistoryDraft`：TXT 未保存草稿回滚到最近一次已保存内容。
- `apps/tracer_android/app/src/main/java/com/example/tracer/ui/viewmodel/ConfigViewModel.kt`
  - `selectedFileContent + editableContent`：已保存内容与草稿分离；`discardUnsavedDraft` 负责回滚。
  - 导入导出流程入口保留在 ViewModel，执行逻辑下沉至 `ConfigBundleTransferUseCase`。
- `apps/tracer_android/app/src/main/java/com/example/tracer/ui/screen/TracerScreen.kt`
  - 标签页切换与 `ON_STOP` 生命周期事件触发草稿丢弃。

## 震动反馈 (Haptic Feedback)

目前支持震动反馈的按键如下：

### 记录页面 (Record Activity)
- **Record Activity 按钮** (`RecordScreen.kt`)
  - **触发时机**: 点击 "Record Activity" 按钮进行活动记录时。
  - **震动类型**: `HapticFeedbackType.LongPress` (长按震动效果)。
  - **交互目的**: 为用户提供明确的记录成功触觉反馈。

## Report：图表模式（两阶段实现-阶段二）

### 目标
- 在 `Report` 页结果区支持 `文字 / 图表` 两种展示模式。
- 默认 `文字` 模式，保持原有 Markdown 报表/统计/树结果显示行为。
- `图表` 模式使用 `Compose Canvas` 展示“选定根节点最近 N 天时长”折线图。

### 交互与参数
- 结果区顶部增加 `SingleChoiceSegmentedButtonRow`：`文字` / `图表`。
- `图表` 模式提供参数区：
  - 根节点下拉（含“全部根节点”）
  - 最近天数输入（N）
  - `加载图表` 按钮
- 切换回文字模式后，图表参数和已加载数据会保留。

### 数据桥接
- `QueryGateway` 新增图表查询接口，透传 `root + lookbackDays`。
- Runtime 通过 `query data report_chart` 获取结构化 JSON，并解析为 `chartPoints`。
- 查询状态：
  - `chartLoading=true` 显示加载中
  - 失败时显示 `chartError`
  - 不影响文字模式结果展示

### 代码定位（Kotlin）
- `apps/tracer_android/feature-report/src/main/java/com/example/tracer/ui/screen/QueryReportTabContent.kt`
  - Report tab 顶层容器，编排参数区与结果区。
- `apps/tracer_android/feature-report/src/main/java/com/example/tracer/ui/screen/QueryReportResultDisplay.kt`
  - 文本/图表/错误结果卡片渲染分发。
- `apps/tracer_android/feature-report/src/main/java/com/example/tracer/ui/screen/ReportChartResultContent.kt`
  - 图表模式页面编排入口。
- `apps/tracer_android/feature-report/src/main/java/com/example/tracer/ui/screen/ReportChartParameterSection.kt`
  - 图表参数输入区（根节点、日期模式、日期参数、加载按钮）。
- `apps/tracer_android/feature-report/src/main/java/com/example/tracer/ui/screen/ReportChartVisualizationSection.kt`
  - 图表结果区（错误/空态/折线柱状饼图/平均线/选中点详情）。
- `apps/tracer_android/feature-report/src/main/java/com/example/tracer/ui/screen/ReportMarkdownRenderer.kt`
  - 报表 Markdown 的 Compose 渲染。
- `apps/tracer_android/feature-report/src/main/java/com/example/tracer/ui/screen/ReportMarkdownParser.kt`
  - 报表 Markdown 解析（block + inline）。
- `apps/tracer_android/feature-report/src/main/java/com/example/tracer/ui/viewmodel/QueryReportChartPipeline.kt`
  - 图表查询 use case 流程（缓存、状态组装、trace）。
- `apps/tracer_android/feature-report/src/main/java/com/example/tracer/ui/viewmodel/QueryReportChartParamResolver.kt`
  - 图表参数校验与查询参数解析。
- `apps/tracer_android/feature-report/src/main/java/com/example/tracer/ui/viewmodel/QueryReportChartMappers.kt`
  - core payload 到 domain/render model 的映射。
- `apps/tracer_android/feature-report/src/main/java/com/example/tracer/ui/viewmodel/QueryReportChartModels.kt`
  - 图表 domain/render/trace 模型定义。

### 多语言
- 新增图表模式相关文案已同步到：
  - `apps/tracer_android/feature-report/src/main/res/values/strings.xml`
  - `apps/tracer_android/feature-report/src/main/res/values-zh/strings.xml`
  - `apps/tracer_android/feature-report/src/main/res/values-ja/strings.xml`

### 跨表现层说明（Android vs Windows CLI）
- 本节“图表模式”是 Android 专属表现层能力，建立在同一 core 查询能力之上。
- Windows CLI 当前仍以 `文字分析 + 格式化报告` 为主，不提供图形化折线图渲染。
- CLI 侧定位与差异说明见：`docs/time_tracer/clients/windows_cli/README.md`。
