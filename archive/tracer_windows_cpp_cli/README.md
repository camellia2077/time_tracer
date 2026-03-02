# Archived Legacy C++ CLI Frontend

本目录归档的是 `time_tracer` 程序开发初期使用的手动实现 CLI 库的 C++ 表现层代码。  
它陪伴了 `time_tracer` 项目的初创阶段。

由于其无法享受到现代 Rust（`clap`）带来的卓越类型安全、声明式自动排版和零配置宏能力，并且为坚持“表现层与核心业务严格分离”的 Clean Architecture 原则，该代码库于 **2026-03-02** 被正式废弃，并由 `rust_cli` 全面取代。

**请注意：** 本目录下的代码已不再参与项目的主线 CMake 构建，也不会随核心 DLL 同步更新，仅作为架构演进历史的凭证在此归档纪念。

## 相关历史文档 (Legacy Documentation)

该阶段的详细设计与变更记录已同步归档：
- [Windows CLI 历史版本索引](docs/history.md)
- [历史版本详细记录 (v0.1.0 - v0.2.0)](docs/history/)
- [历史后端实现说明 (Legacy Backend: C++ CLI)](docs/backends/cpp_cli.md)

---
**Author:** [camellia2077](https://github.com/camellia2077)
