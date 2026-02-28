---
description: Build and test tracer_windows_cli after code changes
---

## Scope Policy

- This file is a short execution guide. Open docs on demand.
- Prefer direct search (`rg`) and only read relevant files.

## Required Flow

1. Reuse incremental build dir:
   - `apps/tracer_cli/windows/build_fast`
2. Canonical verify command:
   - `python scripts/run.py verify --app tracer_core --quick`
3. Validate result:
   - `test/output/tracer_windows_cli/result.json` must contain `"success": true`.

## Optional Split Flow (Debugging)

```powershell
python scripts/run.py configure --app tracer_windows_cli --build-dir build_fast
python scripts/run.py build --app tracer_windows_cli --build-dir build_fast
python test/run.py --suite tracer_windows_cli --build-dir build_fast --agent --concise
```

## Guardrails

- Use Python entry commands only (`scripts/run.py`, `test/run.py`).
- Do not delete `build_fast` unless explicitly requested.
- Temporary files go under `temp/`.
- Canonical integration input source is `test/data`; do not add app-private test dataset copies.

## Docs Entry

- Windows CLI docs index: `docs/time_tracer/clients/windows_cli/README.md`
- Structure map: `docs/time_tracer/clients/windows_cli/specs/STRUCTURE.md`
- Output style: `docs/time_tracer/clients/windows_cli/specs/cli-output-style.md`
- Console color: `docs/time_tracer/clients/windows_cli/specs/console-color.md`
- Core stats contracts: `docs/time_tracer/core/contracts/stats/README.md`

