# tracer_windows_cli

Windows CLI delivery entry for `tracer_core`.

This README is intentionally short. Detailed behavior/specs live under `docs/`.

## Quick Commands

```bash
# Canonical integrated verify (recommended)
python scripts/run.py verify --app tracer_core --quick

# Optional split flow
python scripts/run.py configure --app tracer_windows_cli --build-dir build_fast
python scripts/run.py build --app tracer_windows_cli --build-dir build_fast
python test/run.py --suite tracer_windows_cli --build-dir build_fast --agent --concise
```

## Result Files

- `test/output/tracer_windows_cli/result.json`
- `test/output/tracer_windows_cli/result_cases.json`
- `test/output/tracer_windows_cli/logs/output.log`

## Test Input Source

- Canonical integration input source is `test/data`.
- Do not maintain private per-app dataset copies for CLI.

## Docs Entry

- Windows CLI docs index: `docs/time_tracer/clients/windows_cli/README.md`
- Structure map: `docs/time_tracer/clients/windows_cli/specs/STRUCTURE.md`
- Output style: `docs/time_tracer/clients/windows_cli/specs/cli-output-style.md`
- Console colors: `docs/time_tracer/clients/windows_cli/specs/console-color.md`
- Core stats contracts index: `docs/time_tracer/core/contracts/stats/README.md`

## Agent Entry

- `apps/tracer_cli/windows/agent.md`

