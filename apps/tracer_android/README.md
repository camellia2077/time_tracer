# tracer_android

Android host app (`Jetpack Compose + JNI`) for `apps/tracer_core_shell` core runtime.

Current behavior:
- Android app build is driven by Gradle backend.
- Native runtime is built through `runtime/src/main/cpp/CMakeLists.txt`.
- Core capabilities are consumed via JNI/runtime gateway, not by embedding Windows build artifacts.

Runtime config boundary:
- Canonical shared source: `assets/tracer_core/config`
- Android generated runtime copy: `apps/tracer_android/runtime/src/main/assets/tracer_core/config`
- If shared config semantics change, update `assets/tracer_core/config` first, then resync into Android runtime assets.

Build (Python entry, recommended):
```bash
python scripts/run.py build --app tracer_android --profile android_edit
python scripts/run.py build --app tracer_android --profile android_release
```

Verify (recommended profiles):
```bash
python scripts/run.py verify --app tracer_android --profile android_style --concise
python scripts/run.py verify --app tracer_android --profile android_ci --concise
python scripts/run.py post-change --app tracer_android --run-tests always --concise
```

Recommended split:
- Edit loop: use `build --app tracer_android --profile android_edit` for the fastest local APK rebuild path.
- Validation loop: use `verify` / `post-change` only when you want style checks, test suites, or artifact validation.
- Gradle local performance is configured in `gradle.properties` with build cache, configuration cache, and parallel execution enabled.

Test data consistency:
- Canonical test input is `test/data`.
- Android runtime `input/full` assets are auto-synced from `test/data` during `preBuild`.
- Unit/component tests should use small fixtures and avoid depending on large `test/data`.
- Manual check command:
```bash
python scripts/devtools/android/sync_android_input_from_test_data.py --check
```

Outputs:
- `test/output/artifact_android/result.json`
- `test/output/artifact_android/result_cases.json`
- `test/output/artifact_android/logs/output.log`

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
- Icon generation guide: `apps/tracer_android/icon_generation.md`

Agent rules:
- Detailed build/test rules and scan scope are maintained in `apps/tracer_android/agent.md`.
- XML i18n consistency rule:
  - If `strings.xml` keys are added/removed/updated, synchronize all locales in the same change:
    - `res/values/strings.xml` (English baseline)
    - `res/values-zh/strings.xml` (Chinese)
    - `res/values-ja/strings.xml` (Japanese)
