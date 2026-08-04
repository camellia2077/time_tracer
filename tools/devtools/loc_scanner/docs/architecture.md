# LOC Scanner Implementation Layout

This document is for contributors changing the scanner itself. It is not part
of the refactoring workflow for scanned application code.

- `src/loc_scanner/`: scanner implementation and CLI entry point.
  - `cli_app.py`: CLI dispatch, single-language scan execution, and log handling.
  - `profile_scan.py`: profile scan orchestration and report lifecycle.
  - `profile_analysis.py`: pure profile grouping, summaries, labels, and ranking.
  - `classification.py`: shared source-set and test-path classification.
  - `report_writer.py`: report artifact coordination and file persistence.
  - `profile_report_renderer.py`: primary profile Markdown rendering.
  - `comparison_report_renderer.py`: baseline delta Markdown rendering.
  - `reporter.py`: compatibility facade for console output.
  - `line_console_renderer.py`: language and directory scan console output.
  - `profile_console_renderer.py`: profile scan console output.
- `config/`: default TOML configuration, including `scan_lines.toml`.
- `scripts/`: Windows shortcuts; `scripts/lang/` contains single-language
  commands and `scripts/profile/` contains component-profile commands.
- `tests/`: scanner tests.
- `docs/`: scanner usage and configuration documentation.

Keep scanner implementation decisions separate from the architecture decisions
made about the libraries and presentation modules that it scans.

The scanner itself can be inspected with:

```bat
tools\devtools\loc_scanner\scripts\profile\run_loc_scanner.bat
```
