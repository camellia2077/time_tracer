# 原生模块与插件架构 (Native Modules)

TimeTracer 采用动态可扩展架构生成报表，利用 Windows 原生动态链接库 (DLL) 实现插件化部署。

## 1. 插件模型说明

每种报表格式（如 `DayMd`、`MonthTex`）都被编译为一个独立的 DLL 文件。这种设计不仅使主程序的体积保持轻量，还允许用户在不重新编译核心程序的情况下升级或新增输出格式支持。

### 1.1 目录布局
插件必须统一放置在主程序同级的 `plugins/` 目录下：
```text
bin/
├── time_tracer.exe
├── reports_shared.dll
└── plugins/
    ├── libDayMdFormatter.dll
    ├── libMonthTexFormatter.dll
    └── ...
```

## 2. `reports_shared.dll` (核心 SDK)

所有插件都必须依赖名为 **`reports_shared`** 的共享库。
*   **地位**: 充当插件系统的核心 SDK。
*   **内容**: 导出了核心报表配置类 (`DayBaseConfig`, `ProjectNode`) 和通用工具类 (`TexUtils`, `MarkdownUtils`)。
*   **链接**: 插件通过动态链接此库来共享代码，确保了运行时类型系统的一致性。

## 3. 插件加载机制

主程序通过位于基础设施层的 **`DllFormatterWrapper`** 来管理插件生命周期：
1.  **扫描**: 程序启动或执行任务前，会自动扫描 `plugins/` 目录下符合命名约定的 DLL 文件。
2.  **映射**: 利用 `LoadLibraryA` (Windows API) 将 DLL 加载到进程内存空间。
3.  **实例化**: 调用 DLL 中导出的特定工厂函数 `create_formatter`，获取其实现的 `IReportFormatter` 接口指针。

## 4. ABI 兼容性特别警告

> [!WARNING]
> **ABI 稳定性**: 由于系统使用了原生 C++ 虚接口，二进制兼容性对编译器版本和核心层内存模型高度敏感。
> *   若核心层的 `AppConfig` 或 `DailyLog` 结构发生改变，**必须重新编译所有插件 DLL**。
> *   严禁混用由不同编译器（如 MSVC 与 GCC）编译的插件及主程序，否则将导致程序崩溃。

## 5. 插件描述与元数据
在正式启动耗时的任务前，主程序可以查询插件支持的格式和描述，从而实现基于插件能力的动态帮助文档展示。
