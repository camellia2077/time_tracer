---
description: Build and test log_generator after code changes
---

0. Incremental Build Rule (required)
- Always reuse `apps/log_generator/build_fast` for incremental builds.
- Do not delete `apps/log_generator/build_fast` unless explicitly requested.

1. Canonical One-Command Verify (required)
- `python scripts/run.py verify --app log_generator --build-dir build_fast --concise`
- This is the primary flow: it runs configure/build first, then runs the `test` suite automatically.

2. Optional Split Flow (for debugging only)
- Configure:
  - `python scripts/run.py configure --app log_generator --build-dir build_fast`
- Build:
  - `python scripts/run.py build --app log_generator --build-dir build_fast`
- Run suite:
  - `python test/run.py --suite log_generator --build-dir build_fast --agent --concise`

3. Validate Test Result (required)
- Check `test/output/log_generator/result.json`.
- Expected: `"success": true`.
- If failed, inspect `test/output/log_generator/result_cases.json` for case-level details.

4. Failure Log Locations (required on failure)
- Main aggregated log: `test/output/log_generator/logs/output.log`
- Case logs: `test/output/log_generator/logs/**`

5. Command Policy
- Use Python entry commands only:
  - `python scripts/run.py verify ...`
  - `python scripts/run.py ...`
  - `python test/run.py ...`
- Do not use ad-hoc direct `cmake`/`ninja` commands for this workflow.

6. Completion Criteria
- Verify command exits with code `0`.
- `test/output/log_generator/result.json` exists and has `"success": true`.

7. Temporary File Rule
- If temporary files are needed, store them under repository `temp/`.
- Do not place temporary files under `apps/log_generator/`, `config/`, or `test/` unless explicitly requested.
