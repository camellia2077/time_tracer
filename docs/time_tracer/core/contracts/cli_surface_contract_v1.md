# CLI 表现层语义契约（v1 提案）

## 状态
1. 状态：Draft / Proposal
2. 适用范围：`windows/rust`、`windows/cpp_cli` 等表现层。
3. 目标：让多实现 CLI 共享“语义契约”，而非共享“文案文本”。

## 1. 背景
1. 当前存在多套 CLI 实现（C++ 与 Rust）。
2. 若测试绑定“固定文案”，实现会被无意义耦合，维护成本高。
3. 需要稳定的机器契约（退出码、错误码、能力字段）作为一致性依据。

## 2. 设计原则
1. Core 负责业务语义与稳定机器字段。
2. 表现层负责参数交互、帮助排版、颜色与终端体验。
3. 契约优先验证结构化字段，不以逐字文案为准。
4. ABI 扩展必须增量兼容，不破坏已发布符号。

## 3. 必要契约元素（v1）

### 3.1 退出码契约
1. 表现层必须遵守统一退出码定义（参考 `error-codes.md` 与现有 `AppExitCode`）。
2. 同一错误类别在不同表现层必须落到同一退出码区间。
3. 当实现为某个稳定机器错误码分配专用退出码时，必须在契约文档中明确记录映射。

### 3.2 错误对象契约（当前执行）
1. `ok`（bool）
2. `error_message`（string）
3. `error_code`（string，建议新增或补齐）
4. `error_category`（string，建议新增）
5. `hints`（string[]，可选）

说明：
1. `error_message` 可因实现和语言风格不同而差异。
2. `error_code` 与 `error_category` 必须稳定，供测试和上层自动化判定。
3. 例如 `reporting.target.not_found` 可由 CLI 映射到专用退出码，只要该映射保持稳定。

### 3.3 能力描述契约（当前执行）
1. Core 对外提供“能力描述”字段，避免 CLI 自行推断特性。
2. 能力描述用于：
   - 判断命令是否支持某模式（例如结构化输出）
   - 控制测试分支与降级行为

## 4. Core C ABI 能力接口（已落地）

1. `tracer_core_get_build_info_json(void)`
   - 用途：提供 core 版本、ABI 版本、构建时间等稳定字段。
2. `tracer_core_get_command_contract_json(const char* request_json)`
   - 用途：提供命令语义契约（命令 ID、参数 schema、alias、能力标记）。

建议返回结构（示意）：
```json
{
  "ok": true,
  "error_message": "",
  "contract_version": "1",
  "commands": [
    {
      "id": "query",
      "aliases": [],
      "args_schema_version": "1",
      "supports": {
        "structured_output": true
      }
    }
  ]
}
```

## 5. 版本与兼容策略
1. 契约版本使用独立字段：`contract_version`。
2. 允许新增字段，不允许删除已承诺字段。
3. 字段语义改变必须提升版本并提供迁移说明。
4. `c_abi.md` 仅记录“已生效”符号；提案符号在本文件维护，直到实现落地。

## 6. 测试策略（契约优先）
1. 帮助/版本相关测试：
   - 校验退出码、结构段落、命令 token
   - 不做逐字匹配
2. 错误路径测试：
   - 优先断言 `error_code` / `error_category`
   - 次要断言 `error_message` 关键短语
3. 双实现矩阵：
   - 同一测试同时跑 C++ 与 Rust
   - 以语义字段为判定基准

## 7. 落地顺序建议
1. 先让 Rust CLI 去除对 C++ CLI 的运行时透传依赖。
2. 同步将测试断言迁移为契约断言。
3. 再逐步补齐 Core 的能力描述/错误结构字段。

## 8. 关联文档
1. `docs/time_tracer/core/shared/c_abi.md`
2. `docs/time_tracer/core/errors/error-model.md`
3. `docs/time_tracer/core/errors/error-codes.md`
4. `temp/rs-independence.md`
