# 工程效能演进路线 (Future Roadmap)

本文档列出了 `time_tracer` 项目未来计划引入的工程化工具与质量门禁，旨在进一步强化 Clean Architecture 约束并消除潜在的技术债。

---

## 1. 架构依赖检查器 (Dependency ArchLinter)
解决 Clean Architecture 中可能出现的“依赖倒置”风险。
*   **工具建议**：基于 Clang 的自定义分析脚本或 `cmake-dependency-graph`。
*   **核心作用**：自动化校验分层依赖，例如禁止 `Domain` 层包含 `Infrastructure` 层的头文件。
*   **Modules 收益**：在 C++ Modules 普及后，此工具将转换为对 `import` 拓扑的静态分析，防止模块间的循环依赖。

## 2. 圈复杂度分析 (Cyclomatic Complexity)
弥补 LOC 扫描只能衡量物理长度、无法衡量逻辑混乱度的不足。
*   **工具建议**：`lizard` (轻量化多语言支持) 或 `pmccabe`。
*   **引入逻辑**：设定阈值（如单函数圈复杂度 $\le 15$），强迫开发者在逻辑过密时及时拆分子函数。
*   **长远收益**：有效识别并消除隐形的“屎山”逻辑，降低维护成本。

## 3. 未使用代码检测 (Dead Code Detection)
防止项目在快速迭代中积累“僵尸代码”。
*   **工具建议**：`cppcheck` (开启 `unusedFunction`) 或 Clang `-Wunused` 深度静态分析。
*   **引入逻辑**：定期运行扫描，找出无调用方的接口，提升核心引擎的精简度。
*   **长远收益**：保持 `tracer_core` 的高纯净度，消除“不敢删”的技术顾虑。

## 4. 代码覆盖率门禁 (Code Coverage Gate)
将覆盖率从“黑盒”转为可视化的质量指标。
*   **工具建议**：`gcov` / `lcov` (MinGW) 配合可视化 HTML 报告。
*   **引入逻辑**：在 CI 或本地验证阶段生成报告，标记未执行路径。
*   **重构意义**：在进行 C++ Modules 大规模重构时，它是证明逻辑一致性的终极防线。

## 5. 静态全局状态探测器 (Global State Sentinel)
守护 Clean Architecture 的测试独立性。
*   **工具建议**：利用 `nm` 命令扫描 `.obj` 符号，或自定义符号分析脚本。
*   **引入逻辑**：禁止在非特定区域（如非持久化层）定义非 `const` 的全局或静态变量。
*   **长远收益**：确保引擎保持无状态 (Stateless) 或受控状态，为并发优化打下基础。

## 6. 文档链接一致性检查 (Link Checker)
维护海量设计文档的权威性。
*   **工具建议**：`mlc` (Markdown Link Checker)。
*   **核心作用**：扫描所有 `.md` 文件，确保 `[link](path)` 在物理结构变动后依然有效。
*   **用户价值**：防止开发者在阅读文档时遭遇 404，提升技术沉淀的可用性。
