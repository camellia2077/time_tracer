# tracer_android

Android host app (`Jetpack Compose + JNI`) for `apps/tracer_core` core runtime.

Current behavior:
- Android app build is driven by Gradle backend.
- Native runtime is built through `runtime/src/main/cpp/CMakeLists.txt`.
- Core capabilities are consumed via JNI/runtime gateway, not by embedding Windows build artifacts.

Build (Python entry, recommended):
```bash
python scripts/run.py build --app tracer_android --profile fast
python scripts/run.py build --app tracer_android --profile android_release
```

Verify (recommended profiles):
```bash
python scripts/run.py verify --app tracer_android --profile android_style --concise
python scripts/run.py verify --app tracer_android --profile android_ci --concise
```

Test data consistency:
- Canonical test input is `test/data`.
- Android runtime `input/full` assets are auto-synced from `test/data` during `preBuild`.
- Unit/component tests should use small fixtures and avoid depending on large `test/data`.
- Manual check command:
```bash
python scripts/tools/sync_android_input_from_test_data.py --check
```

Outputs:
- `test/output/tracer_android/result.json`
- `test/output/tracer_android/result_cases.json`
- `test/output/tracer_android/logs/output.log`

Core contract references:
- Core C ABI: `docs/time_tracer/core/contracts/c_abi.md`
- Report chart contract: `docs/time_tracer/core/contracts/stats/report_chart_contract_v1.md`
- Core stats semantic JSON schema: `docs/time_tracer/core/contracts/stats/json_schema_v1.md`
- Core stats docs index: `docs/time_tracer/core/contracts/stats/README.md`

Android docs:
- Agent onboarding (quick file ownership + edit routing):
  - `docs/time_tracer/clients/android_ui/specs/AGENT_ONBOARDING.md`
- Runtime protocol: `docs/time_tracer/clients/android_ui/runtime-protocol.md`
- Android architecture: `docs/time_tracer/clients/android_ui/architecture.md`
- Feature scope: `docs/time_tracer/clients/android_ui/features.md`

Agent rules:
- Detailed build/test rules and scan scope are maintained in `apps/tracer_android/agent.md`.
- XML i18n consistency rule:
  - If `strings.xml` keys are added/removed/updated, synchronize all locales in the same change:
    - `res/values/strings.xml` (English baseline)
    - `res/values-zh/strings.xml` (Chinese)
    - `res/values-ja/strings.xml` (Japanese)
