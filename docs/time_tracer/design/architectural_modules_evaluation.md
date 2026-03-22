# 架构评估：核心功能模块化与目录重排

本文评估将 `tracer_core` 迁入 `libs/` 目录并进行 C++ Modules 化改造的可行性与收益。

---

## 1. 现状分析 (`libs/` 目录现状)

目前 `libs/` 目录下包含两个核心组件：
*   **`tracer_transport`**：负责 DTO 定义与 JSON 编解码。
*   **`tracer_adapters_io`**：负责文件系统与数据转换的适配层。

**评估结论**：这两个组件目前虽处于 `libs/` 目录，但仍使用传统的 `.hpp/.cpp` 包含模式。它们是 C++ Modules 化的 **第一梯队目标**。
*   特别是 `tracer_transport`，它被几乎所有层级引用。将其改为 `import tracer.transport;` 可以彻底消除 `nlohmann/json` 在全工程中的重复解析开销。

---

## 2. `tracer_core` 迁移至 `libs/` 的收益

### 2.1 语义定位对齐 (Semantics Alignment)
*   **当前问题**：`tracer_core` 位于 `apps/` 目录下。在常规习惯中，`apps` 代表“最终产物”或“可执行程序”。
*   **迁移逻辑**：`tracer_core` 本质上是项目的 **Domain & Application Engine**，它并不产生用户直接运行的 EXE，而是为 `rust_cli` (Windows) 和 `tracer_android` 提供核心能力。
*   **收益**：将其移至 `libs/` 明确了其作为“基础设施/引擎模块”的定位，使 `apps/` 目录专注承载真正的展现层（CLI, Android 等）。

### 2.2 物理边界强化 (Clean Architecture Support)
*   通过 C++ Modules 的 `export module tracer.core;`，我们可以强制要求外界只能访问 `api/` 层导出的符号。
*   **收益**：即使是在 C++ 层面，也能从物理上禁止外界越过 API 直接访问 `domain` 或 `infrastructure` 的内部类，实现最严格的架构约束。

---

## 3. 演进预估 (Evolution Roadmap)

若执行此项评估中的构想，项目结构将演进为：

```text
time_tracer/
├── apps/                 # 展现层 (Thin Clients)
│   ├── rust/
│   ├── tracer_android/
│   └── ...
├── libs/                 # 核心引擎与组件 (The Engine)
│   ├── tracer_core/      # 原 apps/tracer_core (Logic & Domain)
│   ├── tracer_transport/ # DTOs & Codecs
│   └── tracer_adapters/  # IO & Infrastructure
└── ...
```

## 4. 编译性能与静态分析优化
引入 C++ Modules 后的预期技术收益：
*   **消除重复解析开销**：`import std;` 与业务模块的二进制接口（BMI）机制将取代传统的文本替换模式。在全量构建中，可显著减少前端解析标准库及核心依赖（如 JSON 库）的耗时。
*   **AST 规模优化**：静态分析工具（如 Clang-Tidy）在处理模块化代码时，由于能够跳过已编译模块的内部细节，其 AST 构建速度和分析效率预计将得到实质性提升。

## 5. 跨语言集成与工程稳定性
*   **接口可见性约束**：C++ Modules 提供的显式导出机制补充了 C ABI 层级的约束。通过 `tracer.core` 聚合模块，展现层（如 Rust CLI）对引擎能力的调用将通过更加明确的符号边界进行。
*   **构建系统解耦**：将核心逻辑移至 `libs/` 并实现模块化，有助于理清 `tracer_core` 与各平台展现层之间的信赖关系，使得核心引擎的迭代与展现层的更新在构建层面实现更深度的并行化。

---
**Status**: 方案评估中 (Phase 5 预备)
**Evaluator**: Antigravity (AI Architect)
**Date**: 2026-03-02
