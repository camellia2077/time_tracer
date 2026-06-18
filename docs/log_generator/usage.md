# Log Generator - 使用指南

`log_generator` 是仓库内的测试数据生成工具 app，用于批量生成符合当前
canonical TXT 契约的月日志文件。

它的主要用途不是作为最终用户产品，而是为仓库内的验证与对账链路提供稳定、
可批量生成的文本输入数据，例如：

- 生成或刷新 `test/data/**` 使用的月日志样本
- 为 validate / convert / ingest 提供共享 TXT 输入
- 为 query / report / export 的 golden 对账提供上游原始数据

## 在仓库中的位置

主链路可以理解为：

1. `apps/tools/log_generator` 生成 TXT 月日志
2. `test/data/**` 保存跨 CLI / shell / Android 复用的 canonical 输入资产
3. 主程序基于这些 TXT 执行 validate / convert / ingest
4. query / report / export 的稳定结果进入 `test/golden/**`

因此，`log_generator` 并不是一个孤立的小工具。它生成的 TXT 形状如果发生变化，
可能会同时影响：

- 下游 ingest 逻辑
- 查询与报表结果
- golden 基线
- `log_generator` 自己的 suite 与 guard

## 当前目录定位

面向当前代码结构，通常只需要先记住这几块：

- CLI 入口与参数解析：
  - `apps/tools/log_generator/src/main.cpp`
  - `apps/tools/log_generator/src/cli/framework/command_line_parser.cpp`
- 应用编排与并发 workflow：
  - `apps/tools/log_generator/src/application/workflow/workflow_handler.cpp`
- 配置加载：
  - `apps/tools/log_generator/src/application/config/config_handler.cpp`
  - `apps/tools/log_generator/src/infrastructure/config/config.cpp`
- 生成逻辑：
  - `apps/tools/log_generator/src/domain/impl/log_generator.cpp`
  - `apps/tools/log_generator/src/domain/components/day_generator.cpp`
  - `apps/tools/log_generator/src/domain/components/event_generator.cpp`
- 自检统计：
  - `apps/tools/log_generator/src/application/workflow/workflow_monthly_average_stats.cpp`

## 配置来源

程序启动后，会从可执行文件所在目录下读取两份配置：

- `config/activities_config.toml`
- `config/converter/alias_mapping.toml`
- `config/converter/aliases/*.toml`

它们的作用大致是：

- `activities_config.toml`
  - 控制 wake keywords、daily remarks、activity remarks、nosleep 概率等
- `converter/alias_mapping.toml` + `converter/aliases/*.toml`
  - 提供可用于生成的活动 token 来源

当前实现中，生成器使用 canonical converter alias bundle 中收集到的活动
token 作为活动池，从而尽量保证生成出的文本能被下游 ETL / ingest 逻辑识别。

## 命令行参数

`log_generator` 当前使用 options 风格，而不是旧版的
`<start_year> <end_year> <items_per_day>` 位置参数风格。

### 基本格式

```text
log_generator.exe [options]
```

### 主要参数

- `-y, --year <year>`
  - 生成单个年份的数据
- `-s, --start <year>`
  - 生成年份区间的起始年
- `-e, --end <year>`
  - 生成年份区间的结束年，包含该年
- `-i, --items <number>`
  - 每日生成的日志条目数，必须大于等于 `2`
- `-o, --output <dir>`
  - 输出目录，默认是 `dates`
- `--seed <int>`
  - 固定随机种子，用于可复现输出
  - 不传时保持当前非确定性行为
- `--event-style <point|interval|mixed>`
  - 控制事件行输出风格，默认是 `point`
  - 当前已实现 `point` 与 `interval`
  - `mixed` 作为后续阶段能力，当前会显式拒绝
- `-n, --nosleep`
  - 启用“通宵日”生成
- `--monthly-average`
  - 打印生成数据的月均 tracked-time 统计
- `-h, --help`
  - 显示帮助
- `-v, --version`
  - 显示版本信息

### 选择年份的规则

当前 CLI 有两种合法方式：

1. 单年模式：

```powershell
log_generator.exe --year 2025
```

2. 区间模式：

```powershell
log_generator.exe --start 2024 --end 2026
```

不能把 `--year` 和 `--start/--end` 混用。

## 输出结构

程序会在指定输出目录下，按年份创建子目录，并在每个年份目录中生成按月组织的
TXT 文件。

例如：

```text
<output_root>/
  2025/
    2025-01.txt
    2025-02.txt
    ...
    2025-12.txt
  2026/
    2026-01.txt
    ...
```

生成的月文件遵循当前共享 raw TXT 月文件头格式：

```text
y2025
m01

0101
0606w
1353睡觉
```

这里的重点是：

- 第 1 行是 `yYYYY`
- 第 2 行是 `mMM`
- 后续按 day block 组织内容

当前支持两种事件行风格：

- `point`
  - 例如 `1353睡觉`
- `interval`
  - 例如 `0900-1030概率统计`

其中 `interval` 风格当前采用第一阶段实现：

- wake 继续输出为 point event
- 非 wake 活动输出为连续、不重叠的 interval 行

## 示例

### 示例 1：生成单年数据

```powershell
log_generator.exe --year 2025 --output dates
```

### 示例 2：生成区间数据并启用通宵日

```powershell
log_generator.exe --start 2024 --end 2025 --items 8 --nosleep --output dates
```

### 示例 3：生成 interval 风格数据

```powershell
log_generator.exe --year 2025 --items 8 --event-style interval --output dates
```

### 示例 4：使用固定 seed 生成可复现数据

```powershell
log_generator.exe --year 2025 --items 8 --seed 123 --event-style interval --output dates
```

### 示例 5：打印月均 tracked-time 统计

```powershell
log_generator.exe --year 2026 --items 8 --nosleep --monthly-average --output dates
```

## 推荐构建与验证方式

从仓库根目录运行，优先使用统一 Python 入口，而不是手写 `cmake` / `ninja`
命令：

```powershell
python tools/run.py verify --app log_generator --build-dir build_fast --concise
```

这条命令会按当前工具链约定完成 configure / build / suite verify。

如需调试，可拆成：

```powershell
python tools/run.py configure --app log_generator --build-dir build_fast
python tools/run.py build --app log_generator --build-dir build_fast
python tools/run.py verify --app log_generator --build-dir build_fast --concise
```

## 验证结果位置

标准验证输出位于：

- `out/test/artifact_log_generator/result.json`
- `out/test/artifact_log_generator/result_cases.json`
- `out/test/artifact_log_generator/logs/output.log`

预期状态是：

- 验证命令退出码为 `0`
- `result.json` 中包含 `"success": true`

## 进一步阅读

- `apps/tools/log_generator/AGENTS.md`
- `apps/tools/log_generator/README.md`
- `test/README.md`
- `docs/toolchain/test/README.md`
- `docs/toolchain/test/test_layering.md`
