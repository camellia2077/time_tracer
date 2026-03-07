# Core 与 JSON 边界设计理念

日期：2026-03-07

## 1. 文档目的

本文说明 Time Tracer 在当前架构下如何处理 `core` 与 `JSON` 的关系，并把近期 JSON boundary 重构沉淀为长期设计原则。

它回答的不是“项目里哪里用了 JSON”，而是：

1. 为什么 `tracer_core` 不能被设计成 JSON 驱动内核。
2. JSON 在系统里应该出现在哪些层。
3. 哪些层必须坚持 typed model。
4. 为什么这套边界对 Windows CLI、Android，以及未来 AI 模块都更稳定。

---

## 2. 核心立场

Time Tracer 的核心立场是：

**`tracer_core` 是 typed semantic model 驱动的内核，不是 JSON DOM 驱动的内核。**

这意味着：

1. JSON 是**交换格式**，不是 core 内部通用语言。
2. `domain` 与 `application` 处理的是业务语义对象、规则、请求模型、结果模型。
3. JSON 的解析与编码只发生在边界层。
4. 当系统需要支持新的客户端、协议、AI 适配器，优先复用 typed model，而不是把 JSON 结构继续往内核扩散。

---

## 3. JSON 在系统中的正确位置

### 3.1 允许使用 JSON 的层

JSON 允许存在于以下边界层：

1. `libs/tracer_transport`
   - 负责 runtime request/response 的 JSON 协议编码与解码。
2. `libs/tracer_adapters_io`
   - 负责 processed JSON 文件导入导出。
3. `libs/tracer_core/src/infrastructure/serialization/**`
   - 负责 typed log model 与 JSON text 之间的编解码。
4. `libs/tracer_core/src/infrastructure/query/data/renderers/**`
   - 负责把 semantic result 渲染成 `text` 或 `semantic_json`。
5. `apps/*` 宿主边界
   - 例如 C API、JNI、CLI runtime glue。

### 3.2 不允许使用 JSON 的层

以下层不得把 JSON 作为核心输入输出模型：

1. `libs/tracer_core/src/domain/**`
2. `libs/tracer_core/src/application/**`
3. 未来的 `libs/tracer_core_ai/src/domain/**`
4. 未来的 `libs/tracer_core_ai/src/application/**`

这几层的职责是维护语义稳定性，而不是承接协议细节。

---

## 4. Core 如何处理 JSON 输入

程序对 JSON 输入的处理原则是：

**在入口解码，在 core 内部去 JSON 化。**

典型路径如下：

1. 宿主层、transport 层或 adapter 层接收到 JSON。
2. 边界层完成：
   - 语法解析
   - 原始字段提取
   - 结构完整性检查
   - 必要的 schema/shape 校验
3. 边界层把数据转换成 typed request / typed DTO。
4. `application` 使用 typed request 编排流程。
5. `domain` 只处理语义规则，不关心原始 JSON 长什么样。

### 4.1 processed JSON 的处理

对 processed JSON 文件，当前设计是：

1. `libs/tracer_adapters_io/src/infrastructure/io/processed_data_io.cpp`
   负责读取文件文本并解析 JSON。
2. `libs/tracer_adapters_io/src/infrastructure/io/processed_json_validation.*`
   负责原始 JSON 的 shape 校验。
3. `libs/tracer_core/src/infrastructure/serialization/json_serializer.*`
   负责把 JSON text 转成 `DailyLog` 等 typed model。
4. `domain` 不再承担“原始 JSON 结构是否合法”的职责。

这次重构之后，processed JSON 的结构校验已经从旧的 domain JSON validator 中迁出。

---

## 5. Core 如何处理 JSON 输出

程序对 JSON 输出的处理原则是：

**在 core 内部先形成语义结果，在出口编码成 JSON。**

典型路径如下：

1. `domain` 计算业务结果。
2. `application` 组织语义结果模型。
3. `infrastructure renderer` 根据输出模式生成：
   - `text`
   - `semantic_json`
4. 宿主层再把结果返回给 CLI、Android 或未来 AI adapter。

也就是说：

1. JSON 输出不是 core 内部中间态。
2. JSON 输出是 renderer 的最终产物之一。
3. core 不依赖“先拼 JSON 再继续处理”的工作流。

---

## 6. `semantic_json` 的设计定位

`semantic_json` 的长期定位是：

**机器可消费的最终输出格式之一。**

它不是：

1. application 内部协议。
2. use case 的中间状态。
3. core 内部共享数据结构。

### 6.1 为什么这样定义

如果把 `semantic_json` 变成 core 内部通用模型，会带来三个问题：

1. 业务逻辑会开始依赖 JSON 字段布局。
2. renderer 与 use case 的职责会重新耦合。
3. 未来替换协议层或扩展 AI 层时，会被 JSON DOM 反向绑定。

### 6.2 当前落地方式

本轮重构已经把 data query 链路中的内部 `bool semantic_json` 扩散收口为显式 `DataQueryOutputMode`。

这体现了两个设计原则：

1. core 内部讨论的是**输出模式**，而不是“到处传一个 JSON 开关”。
2. `semantic_json` 是 renderer 负责兑现的边界能力，不是内核协议中心。

---

## 7. 为什么 domain / application 必须坚持 typed model

### 7.1 domain 的职责

`domain` 负责：

1. 业务规则
2. 语义校验
3. 约束冲突处理
4. 时间与活动逻辑

它不应该负责：

1. 原始 JSON 结构是否合法
2. 某个字段名是否拼错
3. 某个 JSON 数组是否是协议层格式

### 7.2 application 的职责

`application` 负责：

1. 用例编排
2. ports 调度
3. typed request / typed response 的流转
4. 错误语义归一化

它不应该公开：

1. `nlohmann::json`
2. 伪装成 JSON blob 的 `std::string`
3. 把 `semantic_json` 当成内部交换协议的双轨接口

---

## 8. 这次重构删除了什么

这轮 JSON boundary 重构的设计目标之一，是**不保留兼容债务**。

因此程序已经删除以下旧接口或旧结构：

1. `libs/tracer_core/src/domain/logic/validator/json/rules/json_rules.*`
   - 旧的 domain 层 JSON 结构校验链路。
2. `libs/tracer_core/src/infrastructure/reports/shared/utils/config/config_utils.*`
   - 未继续使用的旧 JSON 配置工具口子。
3. `libs/tracer_core/src/infrastructure/serialization/core/log_codec.*`
   - 旧的内部 JSON codec 头接口。

这些删除对应的设计理念是：

1. 不保留“同一职责的旧接口 + 新接口”双轨并存。
2. 不保留“typed model 与 raw json 同时支持”的过渡接口。
3. 不让旧 JSON 直通链路继续成为未来回退入口。

---

## 9. 为什么这套设计更适合未来 AI 模块

未来如果引入：

1. `libs/tracer_core_ai`
2. `libs/tracer_ai_provider`

推荐设计不是让 AI 模块直接吃 `nlohmann::json` DOM，而是：

1. `tracer_core` 产出 typed semantic result。
2. `tracer_core_ai` 消费 typed semantic result。
3. provider adapter 在边界层处理：
   - API key
   - HTTP
   - provider request/response JSON
   - 重试与超时

### 9.1 这样做的好处

1. AI 模块不被某一个 JSON 库锁死。
2. future provider 适配层可以替换而不影响 core。
3. `semantic_json` 仍可作为 agent 可读输出，但不会反向主导 core 架构。

---

## 10. 当前的强制执行方式

这套设计不是“只写在文档里”，而是已经被工程规则约束。

### 10.1 规则层

`C:\code\time_tracer\.agent\rules\AGENTS.md` 已明确要求：

1. `libs/tracer_core/src/domain/**` 禁止新增 `nlohmann/json` 依赖。
2. `libs/tracer_core/src/application/**` 禁止新增 `nlohmann/json` 依赖。
3. 不允许把 `nlohmann::json` 暴露为 application public I/O。
4. 不允许把 `semantic_json` 当成 application 内部协议。
5. 未来 `libs/tracer_core_ai` 也遵守同样边界。

### 10.2 CMake 边界检查

`apps/tracer_core_shell/cmake/CoreBoundaryRules.cmake` 与
`apps/tracer_core_shell/cmake/CoreTargets.cmake` 已提供：

1. include 级检查
2. 内容级检查

因此：

1. `domain/**`
2. `application/**`
3. future `tracer_core_ai/src/domain/**`
4. future `tracer_core_ai/src/application/**`

都会被扫描，防止 `nlohmann::json` 回流。

---

## 11. 面向后续开发的实践准则

如果后续继续修改 core 与 JSON 相关功能，应遵守以下准则：

1. 新增 JSON 输入时，先放在 transport / adapter / parser 边界解码。
2. 进入 `application` 前，必须转换成 typed request 或 typed DTO。
3. 新增 JSON 输出时，优先通过 renderer 生产，不在 use case 内直接拼 JSON。
4. 不新增“为了兼容旧逻辑而保留 raw json + typed model 双轨”的接口。
5. 如果是 AI 相关功能，优先扩展 typed semantic result，再由 adapter 编码成 provider 需要的 JSON。
6. 如果变更 `semantic_json` 字段语义，优先同步：
   - `docs/time_tracer/core/contracts/stats/report_chart_contract_v1.md`
   - `docs/time_tracer/core/contracts/stats/json_schema_v1.md`
   - `docs/time_tracer/core/contracts/stats/README.md`

---

## 12. 一句话总结

Time Tracer 对 core 与 JSON 的设计理念可以概括为：

**JSON 留在边界，语义留在 core；入口解码，出口编码；内核坚持 typed model，协议层负责 JSON。**
