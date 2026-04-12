# Fixture Index

`test/fixtures/` 保存小型专项测试资产。

它和 `test/data/` 的区别是：

1. `test/data/` 是 canonical、长期复用的完整输入集
2. `test/fixtures/` 是为某一个规则、异常路径或兼容场景准备的小样本

## 推荐结构

1. `text/minimal_month/`
   - 极小但合法的 TXT 月文件
2. `text/invalid/`
   - 非法或损坏的 TXT 输入
3. `config/legacy/`
   - 旧版或兼容场景配置
4. `config/custom/`
   - 小型专用配置样本
5. `exchange/`
   - exchange/import/export 专项小样本

## 当前已落样本

1. `text/minimal_month/2026-01.empty.txt`
   - 最小合法空月
   - 适合测 `yYYYY + mMM` 头解析与空月接受路径
2. `text/minimal_month/2026-01.single_day.txt`
   - 最小合法单日块
   - 适合测单日块解析、最小 happy path、`txt view-day`
3. `text/minimal_month/2026-01.cross_midnight.txt`
   - 最小跨午夜场景
   - 适合测跨天时间顺序与通宵边界
4. `text/invalid/2026-01.missing_month_header.txt`
   - 缺失 `mMM` 的非法输入
   - 适合测结构校验与 malformed source 错误路径
5. `text/invalid/2026-01.bad_time_range.txt`
   - 结构合法但包含同分钟冲突事件
   - 适合测逻辑校验失败路径
6. `config/legacy/alias_mapping.legacy.toml`
   - 最小 legacy alias mapping 样本
   - 适合测 compat / fallback 场景

## 边界

1. 不把整年 canonical TXT 数据放进这里
2. 不把长期对账输出放进这里
3. 不把运行生成的 sqlite、logs、临时导出放进这里
4. 不把可由 `test/data` 正常 ingest 生成的 DB 产物 checked in 到这里
