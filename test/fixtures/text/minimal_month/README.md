# Minimal Month Fixtures

这个目录放最小、合法、可直接被 TXT 链路消费的月文本样本。

适用范围：

1. `txt view-day`
2. core `day-block` 解析
3. 最小 happy-path convert / validate / ingest
4. 跨天与 overnight 相关边界

## Current Fixtures

### `2026-01.empty.txt`

- 含 `y2026` 和 `m01`
- 不含任何 day block
- 用于验证：
  - `yYYYY + mMM` 头可被接受
  - 空月是合法输入
  - 缺少目标 day block 时应返回“未找到”，而不是结构错误

### `2026-01.single_day.txt`

- 含一个最小合法 day block
- 用于验证：
  - 单日块解析
  - 最小 authored event happy path
  - `txt view-day` / `ResolveDayBlock` 能正确返回 day body

### `2026-01.cross_midnight.txt`

- 含一个跨午夜边界的最小样本
- 用于验证：
  - overnight / cross-midnight 相关语义
  - day body 在跨天边界场景下仍能稳定解析
  - 不把跨午夜样本误判成结构错误

## Boundaries

1. 不把整年 canonical TXT 数据放进这里
2. 不把故意非法的 malformed 输入放进这里
3. 不把 golden、sqlite、logs、临时导出放进这里
4. 每个文件只服务 1 个主测试目标，避免一个样本承载过多语义
