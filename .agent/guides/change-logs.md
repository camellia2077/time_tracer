---
description: Markdown 文件修改工作流与 Git 提交规范
---

# Markdown Documents Workflow (Markdown 文件修改工作流)

本文档定义了修改项目内 Markdown 文件（如 `docs/`、`.agent/guides/`、`README.md`）时的标准工作流与 Git 提交信息格式。

## 1. 核心原则 (Core Principles)

- **原子性提交**: 每次提交应聚焦于单一文档主题变更（如：更新历史记录、修复排版错误、新增功能文档）。
- **格式统一**: 所有git message 文件必须遵循 本文件定义的 Git Message 格式。
- **Release-Version**: 确保每次文档变更都携带当前发布的版本号（如 `v0.6.1`）。

## 2. 修改流程 (Workflow Steps)

1.  **Edit**: 修改目标 Markdown 文件。
    - 确保标题层级正确（`#`, `##`）。
    - 检查列表缩进与代码块格式。
2.  **Verify**:
    - **预览检查**: 在编辑器（如 VSCode）中使用 Markdown Preview 查看渲染效果。
    - **链接检查**: 确认内部引用链接有效。
    - **表格对齐**: 确保 Markdown 表格渲染无错位。
3.  **Commit**: 使用下述标准格式提交变更。

## 3. Git Commit Message 规范 (Git Message Format)

提交信息必须包含结构化的 Header、Summary、以及具体的变更分类块（[Added]/[Changed]/[Fixed]）与验证说明。

### 模板结构

```text
<type>: <subject>

[Summary]
<详细说明变更的内容与背景原因。>

[Added]
- <列出新增的文档章节或文件。>

[Changed]
- <列出修改的内容、结构调整或格式优化。>

[Fixed]
- <列出修复的错别字、坏链或过时信息。>

[Verification]
- <验证方式说明，例如：预览渲染、链接测试。>

Release-Version: <当前发布版本号>
```

### 字段说明

- **<type>**: 文档修改统一使用 `docs`。
- **<subject>**: 简短总结（建议 50 字符以内）。
- **[Summary]**: (必填) 完整的变更描述。
- **[Added]/[Changed]/[Fixed]**: (选填) 根据实际修改内容选择分类块。如果是纯格式重构，可仅写 `[Changed]`。
- **[Verification]**: (必填) 说明如何验证了文档的正确性。
- **Release-Version**: (必填) 当前版本号，例如 `v0.6.1`。

## 4. 示例 (Example)

```text
docs: Update history style guide formatting

[Summary]
Standardize the version header format and categorization rules in the history style guide to match recent project conventions.

[Changed]
- Enforce `## [vX.Y.Z] - YYYY-MM-DD` header format.
- Merge custom categories into standard KAC (Added/Changed/Fixed).

[Verification]
- Previewed `history-style-guide.md` in VSCode.

Release-Version: v0.6.1
```