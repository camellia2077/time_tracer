# Cursor 无人值守运行说明（time_tracer）

## 脚本文件

- 默认跑到完成：`apps/time_tracer/scripts/agent/run_tidy_all_unattended.ps1`
- 按轮次运行：`apps/time_tracer/scripts/agent/run_tidy_all_unattended_rounds.ps1`

## 前置条件

1. 在 PowerShell 中运行（建议 `pwsh`）。
2. Cursor CLI 可用：`cursor --version` 能输出版本。
3. 当前目录在仓库根目录（`time_tracer_cpp`）。
4. 工作流文件存在：`.agent/workflows/tt-tidy-all.md`。

## 直接跑到完成（推荐）

```powershell
pwsh -File apps/time_tracer/scripts/agent/run_tidy_all_unattended.ps1
```

## 只跑 N 轮

```powershell
pwsh -File apps/time_tracer/scripts/agent/run_tidy_all_unattended_rounds.ps1 -Rounds 10
```

参数说明：

- `-Rounds 0`：无限轮，直到完成（默认）
- `-StableLimit 3`：连续 3 轮无进展就停止
- `-SleepSeconds 2`：轮次之间等待 2 秒
- `-PromptFile <path>`：覆盖默认工作流文件

## 完成判定（硬约束）

只有同时满足下面两条才会退出成功：

1. `apps/time_tracer/build_tidy/tasks/` 下没有任何 `task_*.log`
2. `test/output/time_tracer/result.json` 的 `success` 为 `true`

## Cursor 命令兼容

脚本会自动选择命令：

1. `CURSOR_AGENT_CMD`（若你手动设置了）
2. `cursor-agent`
3. `cursor`

如果检测到是 `cursor`，会自动用：

```powershell
cursor agent -p "<prompt>"
```

## 退出码

- `0`：完成
- `3`：达到 `-Rounds` 上限
- `4`：连续无进展达到 `-StableLimit`
- 其他：Cursor 命令执行失败
