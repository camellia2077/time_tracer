# log_generator

CLI app for generating test log datasets used by validation pipelines.

This README is the fallback build/test guide when `apps/log_generator/agent.md`
is unavailable.

## Build/Test Entry Policy

Run from repository root and prefer Python entry commands:

- Daily one-command flow:
  - `python scripts/run.py post-change --app log_generator --run-tests always --build-dir build_fast --concise`
- Milestone/release flow:
  - `python scripts/run.py verify --app log_generator --build-dir build_fast --scope batch --concise`
- Other operations:
  - `python scripts/run.py ...`

Do not use ad-hoc direct `cmake`/`ninja` commands for this workflow.

## Incremental Build Rule

- Reuse `apps/log_generator/build_fast` for incremental verification.
- Do not delete `apps/log_generator/build_fast` unless explicitly requested.

## Canonical Verify Command

```powershell
python scripts/run.py post-change --app log_generator --run-tests always --build-dir build_fast --concise
```

This command is the primary flow: configure/build first, then run the
`log_generator` suite.

## Optional Split Flow (Debug Only)

```powershell
python scripts/run.py configure --app log_generator --build-dir build_fast
python scripts/run.py build --app log_generator --build-dir build_fast
python scripts/run.py verify --app log_generator --build-dir build_fast --scope artifact --concise
```

## Result Files and Logs

- State file: `apps/log_generator/build_fast/post_change_last.json`

Required result files:

- Summary: `test/output/artifact_log_generator/result.json`
- Case details: `test/output/artifact_log_generator/result_cases.json`

Expected result: `"success": true` in `result.json`.

Failure triage logs:

- Aggregated log: `test/output/artifact_log_generator/logs/output.log`
- `test/output/artifact_log_generator/logs/**`

## Completion Criteria

- Verify command exits with code `0`.
- `test/output/artifact_log_generator/result.json` exists and reports `"success": true`.

## Notes

- If temporary files are needed, store them under repository `temp/`.
