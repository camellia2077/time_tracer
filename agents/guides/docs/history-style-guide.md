---
description: Agent 专用发布历史模板
---

# Release History Template

本文件只定义 agent 编写版本历史或发布说明时必须遵守的最小规则。

## Hard Rules

- 最新条目必须写在最前面
- 版本标题格式必须为 `## [vX.Y.Z] - YYYY-MM-DD`
- 日期必须使用 ISO 8601：`YYYY-MM-DD`
- 分类只使用以下几类：
  - `### 新增功能 (Added)`
  - `### 技术改进/重构 (Changed/Refactor)`
  - `### 修复 (Fixed)`
  - `### 安全性 (Security)`
  - `### 弃用/删除 (Deprecated/Removed)`
- 列表统一使用 `* `
- 空分类不要保留
- 条目应以动词开头，简短直接
- 文件名、命令、路径、配置键统一使用反引号
- 代码、配置或元数据中的版本号递增不写入 history 条目；history 标题中的目标版本号仍按标题格式保留
- UI 组件名称统一使用产品当前英文界面中显示的原文，不混用中文翻译或中英混合名称；优先参考对应 Android 模块的 `values/strings.xml`
- UI 层级必须写清楚：`Files` 是 tab，`Editor` 是 `Files` tab 中的 card，`More edits` 是该 card 打开的 sheet；不要用“Files 编辑器”代替这些名称
- 只有在界面没有明确显示名称时，才使用英文行为描述；不要为组件临时创造中文名称

## Template

```md
## [vX.Y.Z] - YYYY-MM-DD

### 新增功能 (Added)
* 新增 `<feature or file>`

### 技术改进/重构 (Changed/Refactor)
* 重构 `<module or workflow>`

### 修复 (Fixed)
* 修复 `<bug or regression>`

### 安全性 (Security)
* 修复 `<security issue>`

### 弃用/删除 (Deprecated/Removed)
* 删除 `<removed item>`
```

## Writing Rules

- 只写用户可感知或工程上重要的变化
- 同类改动尽量合并表达
- 描述 UI 时保留产品当前英文界面中的原文，例如 `Files` tab、`Editor` card、`More edits` sheet 和 `Raw month TXT` page
- 直接描述最终可见结果，不记录同一版本内的中间重命名、控件迁移或实现顺序
- 涉及目录迁移时，明确写出路径
- 若涉及配置格式、构建方式变化，应明确写出旧口径与新口径
- 仅因发布或构建而修改版本号时，不在 history 中重复记录该版本号变更

## Generic Example

```md
## [vX.Y.Z] - YYYY-MM-DD

### 技术改进/重构 (Changed/Refactor)
* 重构 `src/module/` 目录结构，统一默认装配入口。

### 修复 (Fixed)
* 修复 `YYYY-MM` 日期解析与校验不一致问题。

### 弃用/删除 (Deprecated/Removed)
* 删除旧配置格式与兼容加载链路。
```
