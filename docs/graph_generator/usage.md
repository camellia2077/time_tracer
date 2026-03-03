# Graph Generator - 使用指南

`graph_generator` 是一个基于 Python 的数据可视化工具，能够读取 `Time_Master` 生成的数据库，并创建多种图表，帮助您直观地分析时间数据。

## 程序结构

```
graph_generator/
├── main.py                     # 命令行程序入口
├── main_input.py               # input交互
├── db_access.py                # 数据库查询
|
├── configs/
│   ├── heatmap_colors.json     # 热力图颜色配置
│   └── timeline_colors.json    # 时间线和柱状图颜色配置
|
└── modules/
    ├── day_analyzer.py         # 负责处理“逻辑日”数据
    ├── heatmap_generator.py    # 通用化的热力图生成器
    └── plotters.py             # 包含所有基于 matplotlib 的绘图类
```

## 功能总览

  * 生成每日时间线图 (Timeline)
  * 生成每日活动时长柱状图 (Barchart)
  * 生成项目活动年度/月度热力图 (Heatmap)
  * 生成睡眠状态年度/月度热力图 (Sleep Heatmap)

## 基本命令格式

```bash
python main.py <command> [arguments]
```

> **注意**：程序需要在 PowerShell 或 CMD 中运行，并确保您已安装 `matplotlib` 库。

## 可用命令

| 序号 | 命令格式 | 功能描述 |
|---|---|---|
| 1 | `timeline <YYYYMMDD>` | 为指定日期生成时间线图 |
| 2 | `barchart <YYYYMMDD>` | 为指定日期生成活动时长柱状图 |
| 3 | `heatmap <year> [-p PROJECT]` | 生成项目热力图（年度+月度） |
| 4 | `sleep <year>` | 生成睡眠状态热力图（年度+月度） |
| 5 | `-h, --help` | 查看使用帮助 |
| 6 | `-v, --version` | 查看程序版本 |

## 使用示例

### 1\. 生成时间线图

为指定日期 `20250624` 生成一张详细的时间分布图。

```bash
python main.py timeline 20250624
```

### 2\. 生成柱状图

为指定日期 `20250624` 生成一张展示各项活动总时长的柱状图。

```bash
python main.py barchart 20250624
```

### 3\. 生成项目热力图

  * **默认项目 (`mystudy`)**: 为 `2025` 年生成 `mystudy` 项目的年度和月度活动热力图。
    ```bash
    python main.py heatmap 2025
    ```
  * **指定项目 (`meal`)**: 为 `2025` 年生成 `meal` 项目的年度和月度活动热力图。
    ```bash
    python main.py heatmap 2025 -p meal
    ```

### 4\. 生成睡眠状态热力图

为 `2025` 年生成一张展示每日睡眠状态（是/否）的年度和月度热力图。

```bash
python main.py sleep 2025
```

### 5\. 查看帮助

显示所有可用的命令和选项。

```bash
python main.py -h
```

### 6\. 查看版本

显示程序的当前版本信息。

```bash
python main.py -v
```
