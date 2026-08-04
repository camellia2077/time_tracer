# Clang-Tidy 重构约束

本文档是 `tools/toolchain/commands/tidy/` 与
`tools/toolchain/commands/clang/` 的重构决策标准。

它补充 [architecture.md](architecture.md) 的文件地图和
[autofix_policy.md](autofix_policy.md) 的自动修复准入规则，重点说明 tidy
工作流代码如何安全演进。

## 1. 重构目标

tidy 代码不是普通的命令行胶水。它同时维护：

- clang-tidy/clangd 的外部工具边界；
- 结构化 diagnostics 到 source-cluster queue 的转换；
- task、cluster、scan generation 和 source fingerprint 的一致性；
- 自动修复、focused re-check、构建和归档之间的状态流转。

因此，重构的验收标准不是文件变小，而是：

1. 每个阶段有明确 owner；
2. 状态和 task identity 仍由原来的模型维护；
3. 外部工具适配与 tidy 工作流保持单向依赖；
4. 失败恢复、stale 检测和 re-check 边界更容易测试；
5. 没有新增重复模型、兼容层或隐式状态。

## 2. 当前责任边界

```text
commands/clang/
  纯 clang-format / clang-tidy / clangd 调用、协议转换和结果模型
        ↓
commands/tidy/scan/
  扫描输入、结构化结果收集和 invocation 编排
        ↓
commands/tidy/queue/
  task JSON、source cluster、队列 generation 和任务视图
        ↓
commands/tidy/autofix/
  规则识别、编辑计划、语义 rename 和文本编辑应用
        ↓
commands/tidy/execution/
  agent、source-step、refresh、close、build/recheck/archive 工作流
```

### 2.1 `commands/clang/`

这里的代码只负责工具适配：

- 组装 clang 命令和参数；
- 读写 LSP/JSON-RPC 消息；
- 转换 diagnostics、workspace edit 和 invocation result；
- 应用底层文本 edit。

不得在这里加入 queue、cluster、agent budget、task archive 或业务工作流
决策。

### 2.2 `commands/tidy/queue/`

这里拥有 task 和 cluster 的持久化模型。任何 task JSON 的字段、版本、
`scan_id`、`queue_generation`、cluster identity 或视图生成规则变化，都必须
在这里统一处理。

不得让 execution 层自行拼装另一套 task DTO，也不得让 log/toon 阅读视图
成为机器输入。

### 2.3 `commands/tidy/autofix/`

自动修复分为三层：

1. rule：从诊断生成确定性的 `FixIntent`；
2. planner：根据 intent 计算编辑，不直接写工作区；
3. engine：读取文件、合并/冲突检查编辑、写回并生成 `ExecutionRecord`。

rename 必须优先使用 clangd 语义能力。文本 engine 只能处理诊断精确锚定的
局部替换，不能模拟跨作用域或跨文件语义 rename。

### 2.4 `commands/tidy/execution/`

execution 层负责编排，不拥有底层工具协议或第二套 task 模型。

`source_step` 和 `close` 可以保持为工作流 façade；只有在提取出的代码有
独立的状态/持久化/测试边界时，才移动到 `step_internal/` 或独立 service。

## 3. 允许的重构方式

### 3.1 工作流代码

处理大文件时，先按状态阶段确认边界：

```text
resolve current task
  → detect stale source
  → apply or preview fixes
  → build
  → focused re-check
  → refresh / manual stop / archive
  → persist result and next action
```

可以提取：

- 独立的 artifact refresh；
- 独立的 result/state/checkpoint persistence；
- 具有明确输入输出的 re-check 或 queue-head 查询；
- 可独立测试的失败恢复策略。

不要把每个私有方法都变成一个类，也不要为了降低行数把一个有状态的
工作流拆成多个互相传递隐式状态的 wrapper。

### 3.2 自动修复代码

可以把“计划编辑”和“应用编辑”拆开，但必须保持：

- planner 不写文件；
- engine 负责同一文件内编辑排序、重叠检测和写回；
- 每个 intent 最终都有明确的 applied/previewed/skipped/failed 记录；
- dry-run 与 apply 使用相同的编辑计划；
- 失败原因仍来自统一的 reasons/model 契约。

如果拆分后只是把方法从一个文件搬到另一个文件，没有改善 owner、依赖
方向或可测试性，则不应进行该拆分。

## 4. 不变量

重构不得破坏以下约束：

1. 一个 source file 的当前诊断仍属于同一个 source cluster；
2. `task_*.json` 是机器 canonical contract，toon/log 只是阅读视图；
3. task-local 命令只能处理当前 queue generation 的 task；
4. source fingerprint 变化后，必须先 focused re-check 或 refresh，不能直接
   应用旧 task；
5. 自动修复完成后必须经过 build 和 focused clang-tidy re-check；
6. re-check 仍有诊断时，必须刷新当前 cluster 或转人工处理；
7. 只有 re-check 通过且 artifact 清理成功，cluster 才能 archive；
8. `tidy-close` 仍必须执行 final-full、verify 和空队列门禁；
9. 结果、状态和 next action 必须保持可恢复、可追踪。

## 5. 禁止事项

- 不按 LOC 阈值机械拆分；
- 不为了消除扫描热点制造 pass-through wrapper；
- 不在 `commands/clang/` 中加入 tidy queue 或 agent workflow；
- 不恢复 build.log 文本切分、旧 text parser 或 legacy task artifact 兼容；
- 不让 toon/log 重新成为机器输入；
- 不绕过 focused re-check 直接 archive；
- 不把跨文件、跨作用域 rename 实现成正则替换；
- 不复制 task model、state model 或 diagnostics schema；
- 不用全局可变状态传递 batch/cluster 进度；
- 不以“热点数量下降”作为唯一成功标准。

## 6. 重构前后检查清单

### 重构前

记录以下内容：

- 当前文件的责任和变更原因；
- 主要 callers 和依赖方向；
- 要提取的真实边界及其 owner；
- 受影响的 task/state/result contract；
- 现有 focused test 和缺失测试；
- 预期降低的耦合以及可能的失败风险。

### 重构后

至少检查：

- façade 是否仍只负责编排；
- planner 是否没有工作区写入副作用；
- clang adapter 是否没有反向依赖 tidy workflow；
- task JSON、state 和 result 的 schema 是否未意外变化；
- stale、dry-run、失败、manual、archive 路径是否仍有测试；
- 新模块是否有独立且有意义的责任，而不是单纯搬运代码。

## 7. 必须运行的验证

针对 tidy 代码重构，至少运行：

```powershell
python -m pytest tools/tests/platform/tidy -q
python tools/run.py self-test
python -m compileall -q tools/toolchain/commands/tidy tools/toolchain/commands/clang
python -m tools.devtools.loc_scanner --profile tidy --over 200
```

如果修改了 task JSON、结果文件、CLI 参数或 workflow 终态，还要补跑对应
的 contract 测试，并检查 [tidy README](README.md)、[flow.md](flow.md) 和
[autofix_policy.md](autofix_policy.md) 是否需要同步。

## 8. 验收标准

一次 tidy 重构只有同时满足以下条件才算完成：

- 行为契约和 task/state 不变量保持不变，或已明确更新并补测试；
- 责任边界比重构前更清晰；
- 依赖方向更简单或更明确；
- 重点路径有可执行测试；
- tidy profile 的热点变化能够解释，而不是仅仅转移到新文件；
- 没有新增兼容层、重复模型或隐式状态。
