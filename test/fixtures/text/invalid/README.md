# Invalid Text Fixtures

这个目录放结构非法或语义非法的 TXT 小样本。

它们的目的不是覆盖真实完整月份，而是稳定命中某一类失败路径。

适用范围：

1. parser / validator 错误路径
2. CLI / runtime / Android 的 malformed source 回归
3. 逻辑校验失败信息与 diagnostics 稳定性

## Current Fixtures

### `2026-01.missing_month_header.txt`

- 故意省略 `mMM`
- 文件仍保留 year header 和 day marker
- 预期用于命中：
  - month header 缺失
  - parser 拒绝缺少 `mMM` 的源文件
  - validator 报告 `Month header (mMM) is required`

### `2026-01.bad_time_range.txt`

- 结构合法
- 在同一天内放入两条同一分钟的合法活动别名
- 预期用于命中：
  - activity duration 非正值
  - logic validation 中的 duration error
  - runtime 顶层失败消息里的 recent diagnostics

## Boundaries

1. 不把合法样本放进这里
2. 不把多个独立错误揉进同一个文件，避免测试目标漂移
3. 不把依赖临时运行产物的样本放进这里
4. 如果一个文件开始同时命中多类错误，应该拆分成更小的 fixture
