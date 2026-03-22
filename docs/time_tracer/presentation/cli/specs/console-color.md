# Windows Rust CLI 颜色与终端显示

本文档说明 Rust CLI 的颜色来源与调整方式。

## 1. 当前实现

1. Rust CLI 本身默认输出以纯文本为主。
2. Core 通过 severity + UTF-8 文本输出诊断语义，不再导出稳定的 ANSI 常量契约。
3. Windows 控制台 UTF-8 代码页初始化在：`apps/cli/windows/rust/src/main.rs`。

## 2. 颜色源定义

1. 颜色属于表现层职责，建议在 CLI / shell callback / diagnostics sink 渲染。
2. 若需要新增彩色输出，应基于 severity 或显式 style owner 实现，不要重新把 raw ANSI 常量放回 `tracer_core`。

## 3. 调整规则

1. 不要在多处硬编码 ANSI 转义字符串。
2. 若必须新增 Rust 侧彩色输出，先确认不会破坏测试文本断言。
3. 颜色只能增强可读性，关闭颜色后文本仍需完整可读。
4. contract-critical path 应保持 UTF-8 plain text 可读，不依赖 ANSI。

## 4. 快速审计

```powershell
rg -n "DiagnosticSeverity|EmitInfo|EmitWarn|EmitError|LogInfo|LogWarn|LogError" libs/tracer_core/src
rg -n "\\x1b|ansi|color|SetConsoleOutputCP|SetConsoleCP|severity" apps/cli/windows/rust/src
```
