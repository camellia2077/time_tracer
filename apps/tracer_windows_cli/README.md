# tracer_windows_cli

Windows CLI delivery entry.

Current behavior:
- Build from `apps/tracer_windows_cli` is enabled.
- CLI framework/commands/main and `cli_runtime_factory.cpp` live here.
- Core/adapters are reused from `apps/time_tracer` via subdirectory embedding.
- Output executable remains `time_tracer_cli`.

Build (fast verification):
```bash
python scripts/run.py configure --app tracer_windows_cli --build-dir build_fast
python scripts/run.py build --app tracer_windows_cli --build-dir build_fast
```

Integrated suite (core + Windows CLI):
```bash
python scripts/verify.py --app time_tracer --quick
# equivalent suite command:
python test/run.py --suite tracer_windows_cli --build-dir build_fast --agent --concise
```

Suite output:
- `test/output/tracer_windows_cli/result.json`
- `test/output/tracer_windows_cli/result_cases.json`

Agent notes:
- CLI output style guide: `docs/time_tracer/clients/windows_cli/specs/cli-output-style.md`
- Console color reference: `docs/time_tracer/clients/windows_cli/specs/console-color.md`
- Core stats semantic JSON contract: `docs/time_tracer/core/contracts/stats/json_schema_v1.md`
- Core stats docs index: `docs/time_tracer/core/contracts/stats/README.md`
- Core stats capability matrix: `docs/time_tracer/core/contracts/stats/capability_matrix_v1.md`
- Core semantic JSON versioning policy: `docs/time_tracer/core/contracts/stats/semantic_json_versioning_policy.md`
- Adapter reviewer checklist: `docs/time_tracer/core/contracts/stats/adapter_reviewer_checklist.md`
- Android UI i18n button sync spec (moved to docs root): `docs/time_tracer/android_ui/specs/i18n-button-sync.md`
- Android UI preference storage spec (moved to docs root): `docs/time_tracer/android_ui/specs/preference-storage.md`
