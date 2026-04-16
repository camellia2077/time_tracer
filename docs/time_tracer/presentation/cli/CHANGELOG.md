# Windows Rust CLI 更新日志

本文档记录 `tracer_windows_rust_cli` 的用户可见变更。

---

## [0.2.0] - 2026-03-02

### Changed

- Windows CLI 文档体系重写为 Rust-only。
- 主线描述统一为 `tracer_windows_rust_cli`（去除旧双轨表述）。
- 结构与输出规范补充了 Agent 快速定位路由。

### Kept

- 命令能力仍由 `tracer_core.dll` C ABI 提供。
- 测试入口仍为 `artifact_windows_cli`，套件目录为 `tracer_windows_rust_cli`。

---

## [0.1.0] - 2026-02-24

### Added

- Rust 版 Windows CLI 进入主线（`clap` 参数模型 + C ABI 调用）。
- `crypto` 命令支持进度反馈。
- `chart` 命令支持 HTML 图表导出（line/bar/pie/heatmap-year/heatmap-month）；其中 `pie` 现表示 period root breakdown。
