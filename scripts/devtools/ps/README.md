# REUSABLE SQUASH FIRST PARENT

`reusable_squash_first_parent.ps1` 是一个增强型的 Git Commit Squash 工具，专门用于处理 `--first-parent` 历史树中的提交合并。

## 功能特性

- **连续提交合并**：支持将一组连续的提交合并为一个。
- **元数据保留**：合并后的提交将自动继承合并组中最后一个提交的作者信息和日期。
- **自动后续重播**：合并完成后，工具会自动将其后的所有提交重新应用（Cherry-pick）到新的基准上，保持后续历史完整。
- **安全备份**：自动创建以 `backup/squash-` 为前缀的备份分支，确保操作安全。

## 🤖 Agent 操作规程 (Agent Protocol)

为了确保操作的标准化和安全性，当 AI Agent 需要使用此脚本时，**必须**遵循以下步骤：

1. **环境隔离**：
   - 首先在项目根目录下创建一个 `temp` 文件夹（如果尚不存在）。
   - 将 `reusable_squash_first_parent.ps1` 复制到 `temp` 目录中执行。

2. **内容暂存**：
   - 所有的指令明细、提交消息内容、或者需要批量处理的 Commit ID 列表，都应先写入 `temp` 目录下的 `.txt` 文件中。
   - Agent 应通过读取这些 `.txt` 文件来构造脚本参数或核对操作内容。

3. **路径参考**：
   - 脚本位置：`./scripts/devtools/ps/reusable_squash_first_parent.ps1`
   - 执行位置：`./temp/reusable_squash_first_parent.ps1`
