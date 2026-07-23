# Activity Name Mapping Fixtures

这些 TXT 是跨端复用的专项 fixture，不属于 `test/data/**` 的 canonical 主输入集。

- `2026-01.aliases.txt` 使用 alias activity names。
- `2026-02.canonical.txt` 使用 canonical activity names。
- 两个文件可以分别测试单月转换，也可以按顺序拼接测试多月转换。

转换测试必须保持月份、日期、时间和 remark 不变；只允许改变 event line 中的
activity name。fixture 同时包含 point 和 interval event；wake keyword 是 TXT
结构语义，不应被转换。
