---
description: Build and test log_generator after code changes
---

0. Incremental Build Rule (required)
- Always reuse `apps/log_generator/build_fast` for incremental builds.
- Do not delete `apps/log_generator/build_fast` unless explicitly requested.

1. Canonical One-Command Verify (required)
- `python tools/run.py post-change --app log_generator --run-tests always --build-dir build_fast --concise`
- This is the primary flow: it runs configure/build first, then runs the `test` suite automatically.

2. Milestone/Release Verify (required)
- `python tools/run.py verify --app log_generator --build-dir build_fast --scope batch --concise`

3. Optional Split Flow (for debugging only)
- Configure:
  - `python tools/run.py configure --app log_generator --build-dir build_fast`
- Build:
  - `python tools/run.py build --app log_generator --build-dir build_fast`
- Run suite:
  - `python tools/run.py verify --app log_generator --build-dir build_fast --scope artifact --concise`

4. Validate Test Result (required)
- State: `apps/log_generator/build_fast/post_change_last.json`
- Summary: `test/output/artifact_log_generator/result.json`
- Case details: `test/output/artifact_log_generator/result_cases.json`
- Expected: `"success": true`.

5. Failure Log Locations (required on failure)
- Main aggregated log: `test/output/artifact_log_generator/logs/output.log`
- Case logs: `test/output/artifact_log_generator/logs/**`

6. Command Policy
- Use Python entry commands only:
  - `python tools/run.py post-change ...`
  - `python tools/run.py verify ...`
  - `python tools/run.py ...`
- Do not use ad-hoc direct `cmake`/`ninja` commands for this workflow.

7. Completion Criteria
- Verify command exits with code `0`.
- `test/output/artifact_log_generator/result.json` exists and has `"success": true`.

8. Temporary File Rule
- If temporary files are needed, store them under repository `temp/`.
- Do not place temporary files under `apps/log_generator/`, `config/`, or `test/` unless explicitly requested.
