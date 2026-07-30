# Time Tracer — Agent 工作契约

## 定位与范围

本文件是 Time Tracer 仓库的总规则索引，覆盖共享时间追踪领域/runtime、Android 展示、Windows Rust CLI、C ABI host，以及标准 TXT 测试数据生成器。详细实现、接口和测试说明仍以仓库 `docs/` 与最近的局部 `AGENTS.md` 为准。

## 开始工作前

1. 先确认改动所属目录，再阅读最近的局部入口：
   - `libs/tracer_core/AGENTS.md`
   - `libs/tracer_transport/AGENTS.md`
   - `libs/tracer_core_bridge_common/AGENTS.md`
   - `libs/tracer_adapters_io/AGENTS.md`
   - `apps/android/AGENTS.md`
   - `apps/cli/windows/AGENTS.md`
   - `apps/tools/log_generator/AGENTS.md`
   - `tools/AGENTS.md`
2. 按改动类型阅读对应文档：
   - C ABI：`docs/time_tracer/core/shared/c_abi.md`
   - 报表图表字段或语义：`docs/time_tracer/core/contracts/stats/report_chart_contract_v1.md`，以及该文档点名的 schema/README。
   - 其余领域或产品契约：从 `docs/` 中与目标模块最接近的说明开始。
3. 使用命令帮助确认当前参数，再运行最小相关验证：

```powershell
python tools/run.py -h
python tools/run.py <subcommand> -h
python tools/test.py -h
```

## 全局不变量

- `libs/**`、Android、Windows CLI、C ABI host 和 log generator 共享同一套时间追踪业务语义；改动必须同步受影响的 contracts、tests、generated snapshots 和 documentation。
- 使用 `pwsh` 作为默认 shell 入口；只有用户明确要求时才运行 `.sh` 工作流。
- 默认通过 `python tools/run.py ...` 构建和验证；除非局部规则允许或用户明确要求，不直接调用 CMake、Ninja、Gradle 或 native wrapper。
- 不回退或覆盖无关的用户修改；重构与功能变更保持分离，除非重构是安全实现功能所必需的。
- 临时文件放在仓库 `temp/`，除非用户指定其他位置。
- `config` 是共享配置源；应用本地 config 目录是生成快照。
- exchange-format JSON 不得进入 `libs/tracer_core/src/domain/**` 或 `libs/tracer_core/src/application/**`；这些层不得依赖 `nlohmann/json` 或暴露 `nlohmann::json` 应用层公共类型。

## 按改动范围查文档

| 改动 | 必须先看 |
| --- | --- |
| 核心业务、跨客户端语义 | 最近的 `libs/**/AGENTS.md`、相关 `docs/` contract |
| C ABI 导出符号或签名 | `docs/time_tracer/core/shared/c_abi.md` |
| report chart 字段或语义 | `docs/time_tracer/core/contracts/stats/report_chart_contract_v1.md` 及相邻 schema/README |
| Android 展示 | `apps/android/AGENTS.md` |
| Windows CLI | `apps/cli/windows/AGENTS.md` |
| 日志生成器 | `apps/tools/log_generator/AGENTS.md` |
| 构建、测试、工具链 | `tools/AGENTS.md`、`python tools/run.py -h` 和 `python tools/test.py -h` |

局部规则只在其子树内补充本契约；遇到冲突时，局部规则负责具体路由和验证要求，全局安全、授权和完成条件仍然有效。

## 命令与证据规则

- 命令退出码必须为 `0`；命令输出中的结构化 `success` 字段也必须为成功，否则视为失败并继续调查。
- 优先使用以下证据位置：`out/test/<result_target>/result.json`、`out/test/<result_target>/logs/output.log`。
- 常见结果目标：`tracer_android → artifact_android`、`log_generator → artifact_log_generator`、core/Windows CLI → `artifact_windows_cli`。
- 文档-only 修改可以跳过构建和测试，除非同时改变了代码、配置、脚本、测试、生成物或可执行命令示例。
- 完整矩阵、tidy、打包、发布、设备和 push gate 只在用户要求或适用局部规则要求时运行。

## 变更边界

- 诊断、审查、解释或计划请求只检查证据并报告，不自动实现变更。
- 变更、构建或修复请求可以直接完成明确范围内的本地修改，并运行相关的非破坏性验证。
- 删除、外部写入或显著扩大范围前，需要用户确认。

## 完成条件

- 请求结果和明确验收条件均已满足。
- 行为变更有聚焦回归覆盖，或最终报告指出已能证明行为的现有测试。
- 受影响的 contracts、tests、generated snapshots 和 documentation 与实现一致。
- 适用验证以退出码 `0` 完成，结构化成功字段与退出码一致。
- 规范输入、golden files、生成快照和 runtime 输出只在明确范围内修改。
- 最终报告列出运行的命令、检查的证据、跳过的检查及原因，以及剩余风险或阻塞。
