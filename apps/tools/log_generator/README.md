# log_generator

CLI app for generating test log datasets used by validation pipelines.

This README is the fallback build/test guide when
`apps/tools/log_generator/agent.md`
is unavailable.

## Build/Test Entry Policy

Run from repository root and prefer Python entry commands:

- Daily one-command flow:
  - `python tools/run.py verify --app log_generator --build-dir build_fast --concise`
- Milestone/release flow:
  - `python tools/run.py verify --app log_generator --build-dir build_fast --concise`
- Other operations:
  - `python tools/run.py ...`

Do not use ad-hoc direct `cmake`/`ninja` commands for this workflow.

## Incremental Build Rule

- Reuse `out/build/log_generator/build_fast` for incremental verification.
- Do not delete `out/build/log_generator/build_fast` unless explicitly requested.

## Canonical Verify Command

```powershell
python tools/run.py verify --app log_generator --build-dir build_fast --concise
```

This command is the primary flow: it runs configure/build first, then runs the
`log_generator` suite.

## Optional Split Flow (Debug Only)

```powershell
python tools/run.py configure --app log_generator --build-dir build_fast
python tools/run.py build --app log_generator --build-dir build_fast
python tools/run.py verify --app log_generator --build-dir build_fast --concise
```

## Result Files and Logs

Required result files:

- Summary: `out/test/artifact_log_generator/result.json`
- Case details: `out/test/artifact_log_generator/result_cases.json`

Expected result: `"success": true` in `result.json`.

Failure triage logs:

- Aggregated log: `out/test/artifact_log_generator/logs/output.log`
- `out/test/artifact_log_generator/logs/**`

## Completion Criteria

- Verify command exits with code `0`.
- `out/test/artifact_log_generator/result.json` exists and reports `"success": true`.

## Notes

- If temporary files are needed, store them under repository `temp/`.
