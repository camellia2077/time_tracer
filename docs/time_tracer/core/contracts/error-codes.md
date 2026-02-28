# 错误码定义（Core）

本文给出 TimeTracer Core 错误码规范与第一版映射建议。

## 1. 目标
1. 提供稳定、可机器处理的错误标识。
2. 降低跨端（Windows/Android）错误语义漂移。
3. 支持日志检索、统计和告警聚合。

## 2. 命名规范
1. 使用小写 dot 风格：`domain.category.detail`
2. 保持稳定：错误文案可调整，错误码尽量不改。
3. 新增错误必须新增错误码，不复用“近似码”。

示例：
- `validation.source.unrecognized_activity`
- `validation.time.discontinuity`
- `runtime.io.path_not_found`

## 3. 建议域分组
1. `validation.*`：源数据/逻辑校验
2. `pipeline.*`：流程编排阶段失败
3. `import.*`：入库阶段失败
4. `export.*`：导出阶段失败
5. `runtime.*`：运行时/路径/依赖
6. `io.*`：文件系统读写
7. `config.*`：配置加载与解析

## 4. 与现有校验错误类型的映射建议

来源：`apps/tracer_core/src/domain/logic/validator/common/validator_utils.hpp`

| `validator::ErrorType` | 建议错误码 |
| --- | --- |
| `kFileAccess` | `validation.io.file_access` |
| `kStructural` | `validation.structure.invalid` |
| `kLineFormat` | `validation.line.invalid` |
| `kTimeDiscontinuity` | `validation.time.discontinuity` |
| `kMissingSleepNight` | `validation.sleep.missing_night` |
| `kLogical` | `validation.logic.invalid` |
| `kDateContinuity` | `validation.date.continuity_missing` |
| `kIncorrectDayCountForMonth` | `validation.date.incorrect_day_count` |
| `kSourceRemarkAfterEvent` | `validation.source.remark_after_event` |
| `kSourceNoDateAtStart` | `validation.source.no_date_at_start` |
| `kUnrecognizedActivity` | `validation.source.unrecognized_activity` |
| `kSourceInvalidLineFormat` | `validation.source.invalid_line_format` |
| `kSourceMissingYearHeader` | `validation.source.missing_year_header` |
| `kJsonTooFewActivities` | `validation.json.too_few_activities` |
| `kZeroDurationActivity` | `validation.activity.zero_duration` |
| `kActivityDurationTooLong` | `validation.activity.duration_too_long` |

## 5. 与现有 Diagnostic code 的兼容建议

来源：`apps/tracer_core/src/domain/logic/validator/common/diagnostic.hpp`

当前 `Diagnostic.code` 已存在若干业务码（如 `activity.duration.zero`）。
建议策略：
1. 保留已有码，作为兼容层输入。
2. 在统一出口处映射到规范码（或直接以规范码替换旧码）。
3. 明确旧码到新码的对照，避免双轨长期并存。

## 6. 运行时与流程类错误码（建议起步集）

| 场景 | 建议错误码 |
| --- | --- |
| 配置加载失败 | `config.load.failed` |
| 配置字段非法 | `config.field.invalid` |
| 流程步骤失败 | `pipeline.step.failed` |
| 导入事务失败 | `import.transaction.failed` |
| 导出写文件失败 | `export.write.failed` |
| 路径不存在 | `runtime.io.path_not_found` |
| 运行时依赖缺失 | `runtime.dependency.missing` |

## 7. 管理规则
1. 新增错误码时同步更新本文档。
2. 变更错误码需评估兼容影响（日志检索、自动化脚本、前端展示）。
3. 单测应覆盖关键错误码映射，防止回归。

## 8. 相关代码入口
1. `apps/tracer_core/src/domain/logic/validator/common/validator_utils.hpp`
2. `apps/tracer_core/src/domain/logic/validator/common/diagnostic.hpp`
3. `apps/tracer_core/src/infrastructure/logging/validation_issue_reporter.cpp`
4. `apps/tracer_core/src/domain/ports/diagnostics.hpp`

