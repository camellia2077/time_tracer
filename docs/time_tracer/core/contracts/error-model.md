# 错误模型与日志契约（Core）

本文定义 TimeTracer Core 的统一错误模型、日志落盘策略与分层职责。

## 1. 目标
1. 统一错误输出语义，避免多层重复打印。
2. 统一错误日志落点与命名，支持跨端复用。
3. 将错误收集/落盘能力收敛到 Core，表现层只负责展示。

## 2. 当前契约（已实现部分）
1. Core 通过 `time_tracer::domain::ports` 暴露诊断端口：
   - `IDiagnosticsSink`
   - `IErrorReportWriter`
2. 诊断输出支持 `info / warn / error` 三种严重级别。
3. 错误报告写入支持：
   - 每次运行一个 ISO 文件：`errors-YYYY-MM-DDTHH-MM-SSZ.log`
   - 固定入口：`errors-latest.log`
4. 错误报告默认目录：`<runtime_data_root>/logs/`
   - Windows CLI 约定运行数据根为 `<exe>/output/`
   - 因此默认目录为 `<exe>/output/logs/`

## 3. 统一错误记录模型（建议规范）

建议将错误记录抽象为 `ErrorRecord`（逻辑模型；不要求马上改结构体名）：

| 字段 | 说明 |
| --- | --- |
| `code` | 稳定错误码，供机器识别与统计 |
| `severity` | 严重级别（info/warn/error） |
| `message` | 面向用户的短错误描述 |
| `details` | 技术细节（可选） |
| `source` | 来源定位（文件/行/模块，可选） |
| `timestamp_utc` | UTC 时间戳 |
| `run_id` | 运行实例 ID（可用 ISO 时间衍生） |

## 4. 去重策略（建议规范）
1. 去重应在 Core 执行，而不是在 CLI/UI 层分散实现。
2. 建议去重键：`code + source + normalized_message`。
3. 同类重复只保留一条正文，可附加重复计数（可选）。
4. 摘要层与明细层不得重复渲染同一错误块。

## 5. 日志文件策略
1. 运行启动时创建本次 ISO 日志文件（即便无校验类错误，也保证路径存在）。
2. 运行启动时重置 `errors-latest.log`。
3. 后续错误写入同时追加到：
   - 本次 ISO 日志
   - `errors-latest.log`
4. 终端/UI 展示“本次 ISO 日志路径”。

## 6. 报告路径返回契约
1. Core 应提供“当前运行错误日志路径”查询能力。
2. 调用方只展示 Core 返回路径，不自行拼接日志路径。
3. 当未启用文件写入器时，应返回明确状态（例如 `disabled`）。

## 7. 分层职责
1. Core：
   - 错误模型
   - 错误码
   - 去重
   - 日志落盘与路径返回
2. 表现层（Windows CLI / Android UI）：
   - 渲染文本样式与颜色
   - 交互入口（查看/分享日志）
   - 不重写核心错误语义

## 8. 相关代码入口
1. `apps/time_tracer/src/domain/ports/diagnostics.hpp`
2. `apps/time_tracer/src/domain/ports/diagnostics.cpp`
3. `apps/time_tracer/src/infrastructure/logging/file_error_report_writer.hpp`
4. `apps/time_tracer/src/infrastructure/logging/file_error_report_writer.cpp`
5. `apps/time_tracer/src/infrastructure/logging/validation_issue_reporter.cpp`
6. `apps/time_tracer/src/api/android/android_runtime_factory.cpp`

