# Windows Rust CLI 输出风格规范

本文档定义 Rust CLI 的输出契约，目标是保证测试可稳定解析。

## 1. 总原则

1. 以 `test/suites/tracer_windows_rust_cli/tests/*.toml` 为可执行契约。
2. 新增或改动输出文本时，必须同步修改对应测试断言。
3. 输出保持“可读 + 可脚本解析”，避免同义重复与冗余段落。

## 2. 输出结构

1. 每条命令最多一个总结行（成功或失败）。
2. 详细信息放在过程阶段，不在总结阶段重复输出。
3. 错误信息优先包含：命令名、失败原因、定位提示。
4. 非必要不插入空行；仅在大段分区时使用。

## 3. 帮助与版本

1. 全局 `--help/-h/--version/-v` 由 `clap` 输出。
2. 子命令帮助文本由 `src/cli/mod.rs` 的 `about/help` 字段定义。
3. 对帮助文案的任何改动都视为契约改动，需更新相关测试。

## 4. 错误输出约定

1. 参数错误使用统一入口（`main.rs` + `error/mod.rs`）。
2. 业务错误优先返回稳定退出码，再输出可读文本。
3. 不重复拼接前缀（例如避免 `[INFO] [INFO]` 这类重复）。
4. 出现外部日志路径时，路径文本应完整、可复制。

## 5. 命令特定要求

1. `tree/query/export` 帮助和参数错误文本应保持稳定关键字。
2. `crypto` 输出需保留进度与终态（success/failed/cancelled）语义。
3. `licenses` 仅保留摘要与 `--full` 详细文本。

## 6. 变更检查清单

1. 本地跑：`python scripts/run.py verify --app tracer_core --scope artifact --build-dir build_fast --concise`
2. 检查：`test/output/artifact_windows_cli/result.json`
3. 确认：`success=true` 且无新增非预期字符串回归
