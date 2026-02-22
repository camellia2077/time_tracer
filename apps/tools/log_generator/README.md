# log_generator

CLI app for generating test log datasets used by validation pipelines.

This README is the fallback build/test guide when `apps/log_generator/agent.md`
is unavailable.

## Build/Test Entry Policy

Run from repository root and prefer Python entry commands:

- `python scripts/verify.py ...`
- `python scripts/run.py ...`
- `python test/run.py ...`

Do not use ad-hoc direct `cmake`/`ninja` commands for this workflow.

## Incremental Build Rule

- Reuse `apps/log_generator/build_fast` for incremental verification.
- Do not delete `apps/log_generator/build_fast` unless explicitly requested.

## Canonical Verify Command

```powershell
python scripts/verify.py --app log_generator --build-dir build_fast --concise
```

This command is the primary flow: configure/build first, then run the
`log_generator` suite.

## Optional Split Flow (Debug Only)

```powershell
python scripts/run.py configure --app log_generator --build-dir build_fast
python scripts/run.py build --app log_generator --build-dir build_fast
python test/run.py --suite log_generator --build-dir build_fast --agent --concise
```

## Result Files and Logs

Required result files:

- `test/output/log_generator/result.json`
- `test/output/log_generator/result_cases.json`

Expected result: `"success": true` in `result.json`.

Failure triage logs:

- `test/output/log_generator/logs/output.log`
- `test/output/log_generator/logs/**`

## Completion Criteria

- Verify command exits with code `0`.
- `test/output/log_generator/result.json` exists and reports `"success": true`.

## Notes

- If temporary files are needed, store them under repository `temp/`.
