# Report Output Text Contract v1

状态：`active`  
生效日期：`2026-02-27`  
适用范围：`tracer_core` 报告文本输出链路（core -> windows_cli / android runtime）

## 1. 目标
1. 保证同一格式报告在多端输出行为一致，避免传输层/适配层引入文本变形。
2. 防止出现“端侧额外 trim/replace 导致 markdown 不一致”的回归。

## 2. 端能力矩阵
1. `tracer_core`：支持 `md` / `tex` / `typ`。
2. `windows_cli`：支持 `md` / `tex` / `typ`。
3. `android`：仅支持 `md`（轻量化策略）。

## 3. 强制约束（MUST）
1. 报告成功响应中的 `content/outputText` 必须按原样透传，禁止 `trim()/replace()/normalize`。
2. 业务适配层（JNI/Kotlin/CLI adapter）不得修改报告正文文本内容。
3. 仅允许在 UI 展示解析层做临时格式处理，且不得回写导出文本。
4. 报告文本编码统一为 UTF-8。
5. 换行语义统一为 LF（`\n`），正文必须以单个 LF 结尾（保留末尾换行）。
6. 一致性门禁以“原始字节全等 + `sha256` 全等”为准，不允许通过 CRLF/LF 归一化绕过失败。

## 4. 测试与门禁
1. Android runtime 必须覆盖“成功响应正文原样透传”单测。
2. Core formatter parity 必须持续覆盖 `md/tex/typ` 三格式快照回归。
3. `md` 跨端一致性（Windows CLI vs Android）必须覆盖原始字节一致与 `sha256` 一致。
4. 六类固定样本（`day/month/week/year/recent/range`）必须通过 md golden 字节级校验与渲染结构校验。

## 5. 代码落点
1. Android report translator：`apps/tracer_android/runtime/src/main/java/com/example/tracer/runtime/translators/NativeReportTranslator.kt`
2. Android 输出策略：`apps/tracer_android/runtime/src/main/java/com/example/tracer/runtime/translators/ReportOutputPolicy.kt`
3. Core formatter parity tests：`apps/tracer_core/src/infrastructure/tests/report_formatter/`
4. Android runtime report consistency tests：`apps/tracer_core/src/infrastructure/tests/android_runtime/android_runtime_report_consistency_tests.cpp`
5. 固定样本采样脚本：`scripts/tools/collect_report_markdown_cases.py`
6. 字节级审计脚本：`scripts/tools/report_consistency_audit.py`
7. 渲染结构校验脚本：`scripts/tools/report_markdown_render_snapshot_check.py`
