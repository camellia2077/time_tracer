# Windows Rust CLI 版本历史

本项目 Windows CLI 主线为 Rust 实现（`tracer_windows_rust_cli`）。

## 历程记录

1. [v0.1.0 - Rust Rebirth](history/v0.1.0.md)（2026-02-24）
2. v0.2.0（2026-03-02）：文档与入口统一为 Rust-only
3. 2026-03-07：跟随 core `v0.8.0` 收口 ingest 持久化边界，失败的 `ingest` 不再允许留下新数据库文件，`query/report` 缺库时返回明确错误而非隐式建库

> 说明：历史前端实现已归档，不参与主线构建与测试。
