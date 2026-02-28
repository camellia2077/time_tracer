# 报告格式化引擎 (Report Formatting)

本文描述 Time Tracer 是如何将已经在内存中聚合好的 `ReportData`（包含时长的各项统计和已建好的项目树）拼接转化为具体的渲染输出文件（如 Markdown、LaTeX、Typst 等）的。

权威代码入口位于 `apps/tracer_core/src/infrastructure/reports/` 目录。

## 1. 核心架构与职责分层

在数据被查询和聚合后，它们会以强类型 DTO（Data Transfer Object）的形式存在，例如 `DailyReportData` 或 `MonthlyReportData`。
格式化引擎的任务，就是把这些 DTO 转化为最终的字符串文本流。这里遵循了严格的 **“两段式解耦”** 设计：

### 1.1 `ReportDtoFormatter` (总调度枢纽)
- **文件**: `report_dto_formatter.hpp`
- **职责**: 它并不亲自动手拼字符串！它内部持有一个 `FormatterRegistry` 注册表缓存（支持缓存各类语言的 Formatter 实例）。
- 当外部（如终端命令）想要导出一份 Markdown 报告时，它会拿着 `DailyReportData` 去缓存库里寻找对应的 `IReportFormatter` 实例，并将数据全权委托给该实例去执行 `FormatReportFromView(data)`。

### 1.2 `IReportExportWriter` (物理 IO 输出器)
- **文件**: `report_dto_export_writer.hpp`
- **职责**: 它拿到从 Formatter 返回的一长串完整的 `std::string` 后，负责物理落盘工作。
- 比如，在执行导出命令时，是它决定了将这串 Markdown 字节流写入到 `C:\xxx\2024-01-01.md` 文件中。

## 2. 格式化拼接实战 (以 Markdown 为例)

真正的字符串拼接（Concatenation）和字符画绘制，下放到了极其细分的具象格式化器中。

**源码定位**: `apps/tracer_core/src/infrastructure/reports/daily/formatters/markdown/day_md_formatter_core.cpp`

### 2.1 纯手工字符串流式组装
Time Tracer 没有选用沉重的第三方模板引擎（如 Jinja 或 Mustache），而是利用 C++ 的 `std::string::operator+=` 进行了最高效的基础流式拼接：

```cpp
void DayMdFormatter::FormatHeaderContent(...) const {
  report_stream += "## ";
  report_stream += config_->GetTitlePrefix(); // 例如 "日度报告"
  report_stream += " ";
  report_stream += data.date;
  report_stream += "\n\n";
  // ...
}
```

### 2.2 I18n 兼容设计 (`Config` 提取)
代码中所有的硬编码文案（如 `总时长`、`是否按时睡觉` 等标签），全部从 `DayMdConfig` 中动态读取。
由于引擎支持跨平台，在 Android 平台调用时，它可以下发属于自己的本地化 Config 实例，使得同一套 C++ 拼接引擎能吐出原生对应语言（中/英）的 Markdown。

### 2.3 树节点递归渲染
最引人瞩目的多层级项目时长明细树，是如何画进 Markdown 的？
- `DayMdFormatter` 在末尾会调用 `MarkdownFormatter::FormatProjectTree`。
- 这个通用的 Tree 渲染器（位于 `shared` 目录），会使用深度优先遍历（DFS），并结合深度的缩进等级，输出标准的 Markdown 列表格式，例如：

```markdown
- 工作: 4h 30m
  - 编程: 3h 00m
  - 开会: 1h 30m
- 娱乐: 2h 00m
```

## 3. 多种格式的支持生态

不仅仅只有 Markdown。基于这套严谨的接口规约，Time Tracer 开发了丰富的输出变种，所有实现皆放置在 `apps/tracer_core/src/infrastructure/reports/<维度>/formatters/` 下。

- **`markdown/`**: 输出友好的纯文本，适合快速浏览与移动端跨端显示。
- **`latex/`**: 输出带有严谨预设宏的 `.tex` 源码流，适合直接被编译为极其正式的年度/月度时光审计 PDF 文件。
- **`typst/`**: 支持新兴的高性能排版语言 Typst 所需要的指令流格式。
- **`statistics/`**: 各种小型的策略块，专精于为其他格式提供子面板（如“三日滚动平均值”模块）。

## 4. 总结

Time Tracer 的报表输出采用了 `Data (查询引擎) -> DTO (聚合载体) -> Formatter (字符串渲染) -> Writer (文件落盘)` 的单向数据流。
在最核心的字符串生成部分，依靠极简的原生内存拼接与多态的 `Formatter` 策略模式，确保了哪怕未来需要增加 HTML 报告格式，也仅仅只需新写一个实现 `IReportFormatter` 的类，核心架构无需任何改动。
