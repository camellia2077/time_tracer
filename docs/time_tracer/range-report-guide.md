Range Report Guide (Week Example)
=================================

Overview
--------
Range reports share the same data shape and formatter pipeline (Range).
Week reports are implemented as a Range variant, with ISO 8601 weeks
(`YYYY-Www`) and a Monday–Sunday date range.

This guide explains how to add a new range-style report using **week** as
the example.

Step 1: Data model (Range inheritance)
--------------------------------------
Create a report data type that inherits the Range model:

- `time_tracer_cpp/apps/time_tracer/src/reports/data/model/weekly_report_data.hpp`

This keeps a shared set of fields:
`range_label`, `start_date`, `end_date`, `requested_days`, `total_duration`,
`actual_days`, `is_valid`, `project_stats`, `project_tree`.

Step 2: Querying a single range
-------------------------------
Implement a querier that resolves the input into start/end dates and
binds SQL parameters:

- `time_tracer_cpp/apps/time_tracer/src/reports/data/queriers/weekly/week_querier.hpp`
- `time_tracer_cpp/apps/time_tracer/src/reports/data/queriers/weekly/week_querier.cpp`

For week, parse ISO 8601 and compute start/end:

- `time_tracer_cpp/apps/time_tracer/src/reports/shared/utils/format/iso_week_utils.hpp`
- `time_tracer_cpp/apps/time_tracer/src/reports/shared/utils/format/iso_week_utils.cpp`

Step 3: Batch export for all ranges
-----------------------------------
Add a batch fetcher that aggregates all records by range and builds
project trees:

- `time_tracer_cpp/apps/time_tracer/src/reports/data/queriers/weekly/batch_week_data_fetcher.hpp`
- `time_tracer_cpp/apps/time_tracer/src/reports/data/queriers/weekly/batch_week_data_fetcher.cpp`

For week, group by ISO week, compute:
`range_label`, `start_date`, `end_date`, `actual_days`, and `project_stats`.

Step 4: Report services
-----------------------
Single report is handled by `ReportService` with `BaseGenerator`.
Bulk export uses a dedicated service:

- `time_tracer_cpp/apps/time_tracer/src/reports/services/weekly_report_service.hpp`
- `time_tracer_cpp/apps/time_tracer/src/reports/services/weekly_report_service.cpp`

Register the report in:

- `time_tracer_cpp/apps/time_tracer/src/reports/report_service.hpp`
- `time_tracer_cpp/apps/time_tracer/src/reports/report_service.cpp`

Step 5: Formatters (Range)
--------------------------
Week uses Range formatters (no new DLLs):

- `RangeMdFormatter`
- `RangeTexFormatter`
- `RangeTypFormatter`

Registration happens in `ReportService` via
`GenericFormatterFactory<WeeklyReportData>`.

Step 6: Config models and loaders
---------------------------------
Add week configs to app model and loaders:

- `time_tracer_cpp/apps/time_tracer/src/common/config/app_config.hpp`
- `time_tracer_cpp/apps/time_tracer/src/common/config/report_config_models.hpp`
- `time_tracer_cpp/apps/time_tracer/src/config/internal/config_parser_utils.cpp`
- `time_tracer_cpp/apps/time_tracer/src/config/config_loader.cpp`
- `time_tracer_cpp/apps/time_tracer/src/config/loader/report_config_loader.cpp`
- `time_tracer_cpp/apps/time_tracer/src/config/loader/toml_loader_utils.hpp`

Week configs use the Range label set with `title_template`.

Step 7: Validators
------------------
Add a strategy so week config files are validated:

- `time_tracer_cpp/apps/time_tracer/src/config/validator/reports/strategies/weekly/weekly.hpp`
- `time_tracer_cpp/apps/time_tracer/src/config/validator/reports/strategies/weekly/weekly.cpp`
- `time_tracer_cpp/apps/time_tracer/src/config/validator/reports/strategies/strategy_factory.cpp`

Step 8: CLI support
-------------------
Add `week` to query/export:

- `time_tracer_cpp/apps/time_tracer/src/cli/impl/commands/query/query_command.cpp`
- `time_tracer_cpp/apps/time_tracer/src/cli/impl/commands/export/export_command.cpp`

Supported commands:

- `query week 2026-W05 -f md`
- `export week 2026-W05 -f md`
- `export all-week -f md`

Step 9: Export paths
--------------------
Define output paths for week:

- `time_tracer_cpp/apps/time_tracer/src/application/reporting/generator/report_file_manager.hpp`
- `time_tracer_cpp/apps/time_tracer/src/application/reporting/generator/report_file_manager.cpp`
- `time_tracer_cpp/apps/time_tracer/src/infrastructure/reports/exporter.cpp`

Output naming format:

`week/2026-W05.md` (or `.tex` / `.typ`)

Step 10: Config files
---------------------
Add week configs alongside day/month/period:

App configs:
- `time_tracer_cpp/apps/time_tracer/config/config.toml`
- `time_tracer_cpp/apps/time_tracer/config/reports/week/WeekMdConfig.toml`
- `time_tracer_cpp/apps/time_tracer/config/reports/week/WeekTexConfig.toml`
- `time_tracer_cpp/apps/time_tracer/config/reports/week/WeekTypConfig.toml`

Test configs:
- `my_test/time_tracer_test/config/config.toml`
- `my_test/time_tracer_test/config/reports/week/WeekMdConfig.toml`
- `my_test/time_tracer_test/config/reports/week/WeekTexConfig.toml`
- `my_test/time_tracer_test/config/reports/week/WeekTypConfig.toml`

Notes
-----
- Keep all range reports aligned with `title_template` placeholders:
  `{range_label}`, `{start_date}`, `{end_date}`, `{requested_days}`.
- For week, always normalize input to `YYYY-Www`.
