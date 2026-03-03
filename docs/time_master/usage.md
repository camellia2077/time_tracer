# Time Master - 命令行使用指南

本文档提供了 `Time_Master` C++主程序 (`time_tracker_cli.exe`) 的完整命令行参数和使用说明。

> **注意**：请在 PowerShell 或 CMD 等命令行环境中运行本程序。

### 基本命令格式

```bash
time_tracker_cli <command> [arguments]
```

### 可用命令

| 序号 | 短标签 | 长标签 | 功能描述 |
|---|---|---|---|
| 1 | `-vs <path>` | `--validate-source <path>` | 仅检验源文件的格式 |
| 2 | `-c <path>` | `--convert <path>` | 仅转换文件格式 |
| 3 | `-vo`  | `--validate-output` | 转换后检验输出文件 (需与 `-c` 或 `-a` 配合) |
| 4 | `-a <path>` | `--all <path>` | 执行完整流程(检验源-\>转换-\>检验输出) |
| 5 | `-edc`  | `--enable-day-check`  | 启用对月份天数完整性的检查 |
| 6 | `-p <filepath>` | `--process <filepath>` | 解析单个已格式化的txt文件并导入数据库 |
| 7 | `-q d <YYYYMMDD>` | `--query daily <YYYYMMDD>` | 查询指定日期的统计数据 |
| 8 | `-q p <days>` | `--query period <days>` | 查询过去指定天数的统计数据 |
| 9 | `-q m <YYYYMM>` | `--query monthly <YYYYMM>` | 查询指定月份的统计数据 |
| 10 | `-h` | `--help` | 查看此使用帮助 |
| 11 | `-v` | `--version` | 查看程序版本和更新日期 |