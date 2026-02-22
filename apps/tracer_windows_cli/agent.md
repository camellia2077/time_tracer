---
description: Build and test tracer_windows_cli after code changes
---

0. Incremental Build Rule (required)
- Always reuse `apps/tracer_windows_cli/build_fast` for incremental builds.
- Do not delete `apps/tracer_windows_cli/build_fast` unless explicitly requested.

1. Canonical One-Command Verify (required)
- `python scripts/verify.py --app time_tracer --quick`
- This is the primary flow for Windows CLI verification.
- `tracer_windows_cli` suite maps build target to `apps/tracer_windows_cli`.
- `tracer_windows_cli` suite is the core + Windows CLI integrated suite.

2. Optional Split Flow (for debugging only)
- Configure:
  - `python scripts/run.py configure --app tracer_windows_cli --build-dir build_fast`
- Build:
  - `python scripts/run.py build --app tracer_windows_cli --build-dir build_fast`
- Run suite:
  - `python test/run.py --suite tracer_windows_cli --build-dir build_fast --agent --concise`

3. Validate Test Result (required)
- Check `test/output/tracer_windows_cli/result.json`.
- Expected: `"success": true`.
- If failed, inspect `test/output/tracer_windows_cli/result_cases.json` for case-level details.

4. Failure Log Locations (required on failure)
- Main aggregated log: `test/output/tracer_windows_cli/logs/output.log`
- Case logs: `test/output/tracer_windows_cli/logs/**`

5. Command Policy
- Use Python entry commands only:
  - `python scripts/verify.py ...`
  - `python scripts/run.py ...`
  - `python test/run.py ...`
- Do not use ad-hoc direct `cmake`/`ninja` commands for this workflow.

6. Completion Criteria
- Verify command exits with code `0`.
- `test/output/tracer_windows_cli/result.json` exists and has `"success": true`.

7. Temporary File Rule
- If temporary files are needed, store them under repository `temp/`.
- Do not place temporary files under `apps/tracer_windows_cli/`, `config/`, or `test/` unless explicitly requested.

8. CLI Output Formatting Reference
- If you need to change CLI output text/format/structure, read `docs/time_tracer/clients/windows_cli/specs/cli-output-style.md` first.
- If you need to change CLI colors, also read `docs/time_tracer/clients/windows_cli/specs/console-color.md`.
- Shared ANSI constants are defined by core in `apps/time_tracer/src/shared/types/ansi_colors.hpp`.

9. Cross-App Android Spec References
- Android UI i18n button sync spec is located at repository docs root:
  - `docs/time_tracer/android_ui/specs/i18n-button-sync.md`
- Android UI preference storage spec is located at repository docs root:
  - `docs/time_tracer/android_ui/specs/preference-storage.md`

10. Core Stats Semantic JSON Reference
- Core stats semantic JSON schema:
  - `docs/time_tracer/core/contracts/stats/json_schema_v1.md`
- Core stats docs index:
  - `docs/time_tracer/core/contracts/stats/README.md`
