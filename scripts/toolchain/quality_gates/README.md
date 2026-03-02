# Quality Gates

本目录存放 `verify` 使用的质量门禁脚本实现。

## reporting

- `collect_report_markdown_cases.py`
- `gate_cases_loader.py`
- `report_consistency_audit.py`
- `report_markdown_render_snapshot_check.py`

测试样本来源统一由
`test/suites/tracer_windows_rust_cli/tests/gate_cases.toml` 定义，
上述脚本由 `scripts/toolchain/commands/cmd_quality/verify.py` 编排调用。
