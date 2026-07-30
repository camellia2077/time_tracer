# test/data

Shared integration and end-to-end input data for repository test suites.

## Fixture Range

The current canonical fixture set covers the inclusive ISO date range:

- `2025-01-01`
- `2026-12-31`

In practice this means:

- `test/data/2025/` contains monthly source files for `2025-01` through `2025-12`
- `test/data/2026/` contains monthly source files for `2026-01` through `2026-12`

## Date Input Guidance

When tests exercise CLI report targets:

- prefer canonical ISO targets that resolve inside `2025-01-01` to `2026-12-31`
- compact CLI inputs such as `YYYYMMDD` and `YYYYMM` are allowed only when the
  normalized ISO target still falls inside that fixture range
- do not add report/query/export fixture targets outside this range unless the
  shared test data is expanded in the same change

## Notes

- Android 测试时，TXT 和 `activity_hierarchy/*.toml` 通过
  `tools/scripts/devtools/android/push_test_data.py` 注入应用私有目录，不会在编译时打进 APK。
- The runtime database populated by suite setup stores day/month targets in ISO
  forms such as `YYYY-MM-DD` and `YYYY-MM`.
- If the shared fixture range changes, update this file and the suite-local
  comments that rely on it.
- The current 2025–2026 data was regenerated from the canonical alias assets
  with `log_generator --items 8 --nosleep --seed 123`; the seed keeps the
  checked-in refresh reproducible.
