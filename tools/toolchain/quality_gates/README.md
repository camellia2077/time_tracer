# Quality Gates

本目录存放 `verify` 使用的质量门禁脚本实现。

## insights

- `collect_insights_markdown_cases.py`
- `gate_cases_loader.py`
- `insights_consistency_audit.py`
- `insights_markdown_render_snapshot_check.py`

测试样本来源统一由
`tools/suites/tracer_windows_rust_cli/tests/gate_cases.toml` 定义，
上述脚本由 `tools/toolchain/commands/cmd_quality/verify.py` 编排调用。
