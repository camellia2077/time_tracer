# 输入层解析指南 (Input Parsing Guides)

本目录专门记录 Time Tracer 是如何从文件流中读取纯文本日志，并在内部转化为内存实体的**底层引擎算法**。这是一份纯粹面向核心开发者的架构机理解剖文档。

*(注：如果您是普通用户，只是想知道“年月日”和“活动”应该怎么写才不会报错，请查阅 `docs/time_tracer/user_manual/input_format_cn.md` 这是专门给使用者看的操作手册。)*

---

### 文本解析核心算法 (Text Parsing Algorithm)
**[文本解析核心算法 (Text Parsing Algorithm)](text_parsing_algorithm.md)**
- 揭示了隐藏在 `application/parser/text_parser.cpp` 里的核心秘密。
- 解释了在没有抽象语法树（AST）和重量级正则的情况下，程序是如何依靠**基于流的单遍状态机 (Single-pass State Machine)** 做到最高效的逐行截取的。
- 包含事件时间提取、备注分隔（从主串剥离开）、跨天事件（Continuation）和早起事件（Wake）的试探性环境探测与提取细节。
