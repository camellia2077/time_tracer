# Clang-Tidy Auto-Fix Policy

本文档定义 `tools/toolchain/commands/tidy/autofix/` 的准入边界：

- 哪些 clang-tidy 问题允许写成 Python auto-fix
- 哪些只能停留在 dry-run / suggestion
- 哪些不允许进入 auto-fix
- 新规则进入 auto-fix 前必须满足什么条件

目标不是“尽量多修”，而是“只自动做低风险、可验证、可回放的修改”。

## 1. 基本原则

1. `auto-fix` 只解析 canonical `task_*.json`
   - 不解析 `task_*.toon`
   - 不解析 `task_*.log`
2. `toon` / `log` 是阅读视图，不是机器契约
3. Python auto-fix 只能做确定性替换
4. 所有自动修改都必须能落成结构化 report
5. 规则必须能通过后续 build sanity check 和 clang-tidy re-check 验证

## 2. 为什么机器消费统一走 JSON

- JSON 有稳定解析器，结构清晰，适合作为 source of truth
- TOON 适合人和 agent 阅读，但目前没有官方标准库，也没有成熟、通用的第三方解析生态
- 如果把 TOON 当成机器契约，通常只能依赖项目内 codec 或手写解析
- 手写解析会对展示格式演进非常敏感，稳定性差于 JSON

结论：

- `json` 用来解析
- `toon` 用来阅读

## 3. 允许进入 Auto-Fix 的修改类型

### 3.1 符号重命名

允许，但必须优先使用 clang 相关的语义能力。

硬规则：

- 只要是 rename，就默认必须优先走 clangd / clang rename / 其他等价语义级能力
- 禁止把“正则替换”或“纯文本全局替换”当成 rename 的正式实现
- 只有在语义 rename 已经负责最终改名位置选择时，Python 才能参与前置筛选、命名模板推导、结果校验和报告输出
- 如果一个 rename 规则无法落到 clang 语义能力上，默认不进入 auto-fix apply 路径
- 这类规则最多进入 `suggest` 或 `preview-only`，不能直接自动落盘

当前已支持：

- `readability-identifier-naming`
  - 仅限 rule-driven rename
  - 当前只接受局部 `const` 常量 / 变量场景
  - 新名字必须能由稳定模板推导，例如 `payload -> kPayload`

原因：

- rename 是语义操作，不是纯文本替换
- 直接正则替换容易误伤同名符号、注释、字符串、其他作用域
- rename 的正确性依赖引用关系、声明/定义绑定、作用域边界，这些都不应该由正则承担

### 3.2 单行、局部、确定性的文本替换

允许，但必须满足“定位点明确 + 替换规则固定 + 影响范围很小”。

当前已支持：

- `google-runtime-int`
  - 例如 `long long -> std::int64_t`
  - 例如 `unsigned long long -> std::uint64_t`
- `readability-redundant-casting`
  - 删除同型冗余 `static_cast`
- `google-explicit-constructor`
  - 在目标构造函数前插入 `explicit `
- `readability-use-concise-preprocessor-directives`
  - 例如 `#if defined(_WIN32) -> #ifdef _WIN32`
  - 例如 `#if !defined(FOO) -> #ifndef FOO`

这类规则的共同特点：

- 替换由诊断消息直接确定
- 修改落在单行或极小范围
- 不依赖复杂跨文件语义推断
- 易于做 dry-run diff

### 3.3 受限场景下的块级文本替换

允许，但默认先 preview-only，不直接 apply。

当前已支持：

- `google-build-using-namespace`
  - 把 `using namespace tracer_core::core::dto;`
  - 替换成显式 `using` 声明块
  - 支持 `/application/use_cases/` 与 `/application/pipeline/` 受限目录
  - 其中 `using namespace std::chrono;` 会生成 `using std::chrono::<symbol>;` 声明块
  - 当前默认 `preview_only`

这类规则比单行替换风险更高，所以必须额外限制：

- 目录范围固定
- 旧文本精确匹配
- 新文本由本地分析器稳定生成
- 默认先预览，不直接落盘

## 4. 可以用正则，但只能在这类场景

正则不是禁止使用，但只能作为“受限文本手术”的实现细节，不能作为大范围语义修复手段。

允许：

- 在已命中的诊断行上做锚定匹配
- 从当前行提取冗余 `static_cast<...>(...)` 的内部表达式
- 从诊断消息中提取固定 replacement
- 对已知旧文本做一次性、定长或结构稳定的局部替换

不允许：

- 用正则做任何正式 rename
- 用正则做跨文件 rename
- 用正则扫描整个仓库批量改符号名
- 仅凭字符串相等就替换所有同名标识符
- 用文本替换模拟 clang rename
- 对未被诊断精确锚定的位置做大面积改写
- 依赖脆弱上下文猜测语义

简单说：

- 正则可以做“手术刀”
- 不能做“语义分析器”
- 更不能做“rename 引擎”

## 5. 当前不允许进入 Auto-Fix 的类型

以下类型默认禁止直接写成 Python auto-fix：

- `clang-diagnostic-error`
  - 例如 module not found
  - 例如 include / symbol / compile failure
- 需要跨多个文件协同修改的语义重构
- 需要理解模板、宏展开、条件编译后真实语义的修改
- 需要改 public API / 接口签名 / override contract 的修改
- 可能影响 ABI、序列化格式、协议契约的修改
- 只能靠“猜测作者意图”决定结果的修改
- 改动范围难以静态上界化的批量文本替换
- 没有稳定回放条件的启发式修复

当前典型不建议直接进入 auto-fix 的例子：

- `google-default-arguments`
- 模块导入错误
- 复杂 include 重排
- 复杂函数签名重写
- 需要移动代码块 / 拆函数 / 合并函数的重构

## 6. 新规则进入 Auto-Fix 的准入条件

一个新的 clang-tidy check 只有同时满足以下条件，才适合进 Python auto-fix：

1. 诊断来源稳定
   - check 名称稳定
   - 诊断 message 有可依赖模式
2. 输入稳定
   - 只依赖 canonical `task_*.json`
3. 修改目标可确定
   - 不需要人类补充意图
   - 不依赖模糊猜测
4. 修改范围可控
   - 最好单文件
   - 最好单点或单行
   - 最多小块替换
5. 修改应当幂等
   - 重跑不应重复插入或继续破坏源码
6. 可以生成 dry-run diff
7. 可以在失败时明确报告原因
8. 可以被 build + clang-tidy re-check 验证
9. 必须补测试
   - 命中样例
   - 不命中样例
   - 负例
   - 幂等性
   - report 内容

## 7. 规则实现优先级

新增 auto-fix 时，优先级建议如下：

1. 语义级工具
   - 例如 clangd rename
2. 受诊断锚定的局部文本替换
3. 受限目录下的块替换
4. preview-only suggestion
5. 放弃自动修复，只输出 suggest/report

如果某规则只能通过“大范围正则扫全仓”实现，默认不进入 auto-fix。

## 8. 当前建议继续扩展的方向

优先考虑这类规则：

- 命名规范类，但必须能落到 clangd rename 或强约束模板
- 冗余语法类
  - 冗余 cast
  - 冗余关键字
  - 明确可删的局部样板
- 确定性类型替换
  - 前提是 replacement 集合很小且规则固定
- 明确的一行替换类 include / qualifier 补齐
  - 前提是旧文本和新文本都可确定

谨慎考虑：

- using namespace 类块替换
- include 插入 / 去重
- namespace qualifier 补齐

默认不要碰：

- 编译错误恢复
- module/import 修复
- API 设计类调整
- 控制流重写
- 结构性重构

## 9. 实现位置建议

新增规则时，按下面结构落：

- 规则入口：`tools/toolchain/commands/tidy/autofix/rules/*.py`
- 语义/文本分析辅助：`tools/toolchain/commands/tidy/autofix/analyzers/*.py`
- 执行引擎：
  - 语义 rename：`engines/clangd_rename_engine.py`
  - 文本替换：`engines/text_edit_engine.py`
- 汇总入口：`tools/toolchain/commands/tidy/autofix/registry.py`

如果一个规则还不能满足本文件的准入条件，先放在：

- `tidy-task-suggest`
- 或 report / preview-only

不要直接进 apply 路径。

## 10. 当前已落地规则清单

| check | action kind | engine | 默认模式 | 备注 |
| --- | --- | --- | --- | --- |
| `readability-identifier-naming` | `rename` | `clangd` | apply | 仅限 rule-driven const local rename |
| `google-runtime-int` | `runtime_int` | `text` | apply | 固定类型替换，必要时补 `<cstdint>` |
| `readability-redundant-casting` | `redundant_cast` | `text` | apply | 单行冗余 cast 删除 |
| `google-explicit-constructor` | `explicit_constructor` | `text` | apply | 行前缀插入 |
| `readability-use-concise-preprocessor-directives` | `concise_preprocessor_directive` | `text` | apply | 条件宏写法收敛为 `#ifdef/#ifndef` |
| `google-build-using-namespace` | `using_namespace` | `text` | preview-only | 受限目录块替换（dto + chrono） |

## 11. 一句话准入标准

只有当一个 clang-tidy 问题满足“机器可稳定识别、替换结果唯一、修改范围很小、失败可解释、结果可验证”时，才适合写成 Python auto-fix。
