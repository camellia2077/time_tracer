# Application 层架构：流水线编排与边界适配网关

本篇文档旨在深入剖析 Time Tracer 的“流程控制器” —— `application` 应用层。

作为 Clean Architecture 的核心防腐层，`application` 层向下调用 `domain` 层的纯粹计算逻辑，向外则通过 `ports` 定义契约，将不可预测的外部世界（TXT文件、控制台输入流、SQLite持久化）完全隔离在外。

## 1. 核心职责分解

`application` 层不包含任何复杂的跨天算法（计算均在 `domain`），也不直接触碰任何数据库 C-API（交互均在 `infrastructure`）。它的使命是**调度验证**、**驱动管线**与**协议转换**。

其目录位于：`apps/time_tracer/src/application/`

### 1.1 `pipeline` (流水线大脑)
**核心基建**：`pipeline_manager.hpp`
- 引擎抛弃了早年散装堆砌的阶段函数，引入了基于 `PipelineContext` 传递状态的阶段管线范式。
- 管线由多个隔离的 `PipelineStep` 构成（如：准备环境、输入验证、转换数据、载入存储等）。
- `PipelineManager` 负责高层编排。它一旦发现中间环节某一步返回 `false`（例如 `InputValidatorStep` 发现文件编码异常），就会立刻阻断后续步骤，将错误优雅地沿路返回给外层终端。

### 1.2 `service` (业务调度门面)
**核心基建**：`converter_service.hpp`
- 这是驱动 `domain` 运算引擎的离合器。
- `ConverterService::ExecuteConversion()` 并不直接算数，而是建立循环“读取一行 -> 交给解析器 -> 拿到事件 -> 交给 `DayProcessor`（Domain区） -> 推送至下一天” 的主脑控制流。它在这里维护了那扇至关重要的“**前日与今日滑块窗口**”。

### 1.3 `parser` (降维修表车间)
**核心基建**：`text_parser.hpp`, `memory_parser.hpp`
这是 Application 层非常经典的一个防腐隔离实现：

*   **入向解包 (`TextParser`)**：
    从充满不可控特性的文本文件中剥离有用信息。利用字符比对剥离出前四位是时钟、拦截 `//`、`#`、`;` 为备注项、甚至负责校验 `25:99` 这种非法人造时间并抛掷异常 (`Diagnostic`)。
*   **出向压榨 (`MemoryParser`)**：
    这是为了让 `domain` 对象不再污染底层基础设施。复杂的领域对象 `DailyLog` 在准备入库前，交给 `MemoryParser` 彻底拍扁。它不计算任何时长，仅仅是将 `DailyLog` 里的 `stats` 原封不动复制提取，将跨天产生的空对象用 `nullopt` 填充。最终只产出两份极其枯燥但立即可作入库（Insert）的极简 struct：`DayData` 和 `TimeRecordInternal`（统称为 `ParsedData` 包）。

## 2. 核心架构模式：Ports & Adapters (端口与适配器)

Time Tracer 能完美抽离核心逻辑复用于 Android 和 Windows 的秘密武器，都藏在 `application/ports` 这个文件夹内。

**原则：依赖倒置**
`application` 层如果要写数据库、读取配置文件或者打印日志，**绝对不允许去 `#include <sqlite3.h>` 或者直接调用 `std::cout`**。

它只做一件事：**声明需求声明单（Ports）。**

举例：
在 `application/ports/` 中你只会看到纯虚基类：
```cpp
class IProcessedDataStorage {
 public:
  virtual ~IProcessedDataStorage() = default;
  virtual auto Save(const ParsedData& data) -> bool = 0;
};
```
这就是端口。Application 只管调用 `Save()`。

至于这个 `Save` 到底是写入了 Windows 桌面的 `time_master.db`，还是通过安卓 JNI 传给了 Kotlin 写进 Android Room？`application` 毫不知情，也毫不关心。这些具体实现都被推配到外围的 `infrastructure` 组件或者 Android 特供 Adapter 中。

### 2.1 关键的 Port 契约一览
- **输入域契约**：
  `IIngestInputProvider` (负责提供需要解析的原始数据内存块或流，由外部决定是读文件还是传参)。
  `IConverterConfigProvider` (负责提供 `domain` 计算需要的配置文件映射对象)。
- **输出域契约**：
  `IProcessedDataStorage` (负责处理转换好的 `ParsedData` 数据群落，持久化落库)。
  `IReportExportWriter` (报表导出服务时，负责把 Markdown/Tex 文本存到具体的系统目录去)。
- **监控契约**：
  `IValidationIssueReporter` / `Logger` (当架构深层捕捉到异常时，上报问题，由外部宿主决定是打印控制台红字，还是弹出一个 Toast)。

## 3. 典型的数据流向穿越剧

当您在终端敲下导入命令的那一刻：
1. **外部世界 (`infrastructure`)** 首先被唤起，它实例好了一个具体的 SQLite Writer 和文本读取器，作为参数注入（依赖注入）给了 `application` 层。
2. **`PipelineManager` (Application 层)** 接过了指挥棒，开始逐个激活 Step。
3. 当到达转换阶段，**`ConverterService` (Application)** 调动 **`TextParser`** 从被注入的 Reader 中咬下一行行字符串。
4. 字符串变成了 `RawEvent` 后，被立刻推向中央深谷 —— **`DayProcessor` (Domain 层)`** 进行复杂的跨天加持和时长计算。
5. 满载而归的领域对象经过 **`MemoryParser` (Application)** 的无骨压制，变成了僵硬的 `ParsedData` 包。
6. 最后，`PipelineManager` 通过调用 `IProcessedDataStorage->Save(...)` **(Ports 接口)**，将数据托付给了不知道躲在哪里的数据库连接器。

## 4. 总结
`application` 层就像是一个高技术含量的前台大厅和邮政分拣中心。它负责在纯净的后端（Domain）与肮脏多变的外部世界之间来回摆渡、翻译、转接，确保没有外部的污渍能够沾染到核心的时间统筹算法。
