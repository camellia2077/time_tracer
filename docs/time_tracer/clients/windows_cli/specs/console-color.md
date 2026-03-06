# Windows Rust CLI 颜色与终端显示

本文档说明 Rust CLI 的颜色来源与调整方式。

## 1. 当前实现

1. Rust CLI 本身默认输出以纯文本为主。
2. pipeline 等日志中的 ANSI 颜色主要来自 `tracer_core`。
3. Windows 控制台 UTF-8 代码页初始化在：`apps/tracer_cli/windows/rust_cli/src/main.rs`。

## 2. 颜色源定义

1. Core ANSI 常量定义：`libs/tracer_core/src/shared/types/ansi_colors.hpp`
2. 若要统一调整全局颜色语义，应修改上面的 core 常量与调用点。

## 3. 调整规则

1. 不要在多处硬编码 ANSI 转义字符串。
2. 若必须新增 Rust 侧彩色输出，先确认不会破坏测试文本断言。
3. 颜色只能增强可读性，关闭颜色后文本仍需完整可读。

## 4. 快速审计

```powershell
rg -n "ansi_colors|\\033\\[|kRed|kGreen|kYellow|kCyan|kGray" libs/tracer_core/src
rg -n "\\x1b|ansi|color|SetConsoleOutputCP|SetConsoleCP" apps/tracer_cli/windows/rust_cli/src
```
