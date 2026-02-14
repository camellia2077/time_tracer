# Summary

<!-- Briefly describe what changed and why. -->

## Platform Sync Checklist (Required)

- [ ] Port contract changed (`application/ports/*`)?
- [ ] If yes, Windows implementation updated.
- [ ] If yes, Android implementation updated (or placeholder status explicitly updated in contract doc).
- [ ] `docs/time_tracer/platform_sync_contract.md` status table updated.
- [ ] Windows checks passed:
  - [ ] `python scripts/run.py build --app time_tracer`
  - [ ] `ctest --test-dir apps/time_tracer/build_fast --output-on-failure`
  - [ ] `python test/run.py --suite time_tracer --agent --build-dir build_fast --concise`
- [ ] Android build check passed:
  - [ ] `./gradlew :app:assembleDebug` (or equivalent in Android project)
- [ ] `scripts/check_platform_sync.ps1` passed for this change.

## Validation Notes

<!-- Paste key command outputs or a short summary. -->
