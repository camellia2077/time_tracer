# tracer_android i18n Button Text Sync

This document defines where to update multilingual resources when a button text is added, changed, or removed.

## Scope

Applies to any visible button text in:

1. `app` module
2. feature modules (`feature-data`, `feature-record`, `feature-report`)

Do not hardcode user-visible button text in Compose code.

## Language Baseline

For every button text key, keep these three locales aligned:

1. English: `values/strings.xml`
2. Chinese: `values-zh/strings.xml`
3. Japanese: `values-ja/strings.xml`

## Where To Modify

### App module buttons

Update all:

1. `apps/tracer_android/app/src/main/res/values/strings.xml`
2. `apps/tracer_android/app/src/main/res/values-zh/strings.xml`
3. `apps/tracer_android/app/src/main/res/values-ja/strings.xml`

### Feature module buttons

Update all `values*` files inside the same feature module:

1. `apps/tracer_android/feature-data/src/main/res/values/strings.xml`
2. `apps/tracer_android/feature-data/src/main/res/values-zh/strings.xml`
3. `apps/tracer_android/feature-data/src/main/res/values-ja/strings.xml`
4. `apps/tracer_android/feature-record/src/main/res/values/strings.xml`
5. `apps/tracer_android/feature-record/src/main/res/values-zh/strings.xml`
6. `apps/tracer_android/feature-record/src/main/res/values-ja/strings.xml`
7. `apps/tracer_android/feature-report/src/main/res/values/strings.xml`
8. `apps/tracer_android/feature-report/src/main/res/values-zh/strings.xml`
9. `apps/tracer_android/feature-report/src/main/res/values-ja/strings.xml`

If a feature currently has only `values/strings.xml`, add matching `values-zh` and `values-ja` entries when introducing new visible button text.

## Change Rules

### Add button text

1. Add a new string key in English.
2. Add the same key in Chinese and Japanese.
3. Use the key in Compose (`stringResource(...)`), not literal text.

### Update button text

1. Keep key name stable if semantics are unchanged.
2. Update all locales in the same change.

### Remove button text

1. Remove UI usage first.
2. Remove the key in all locales.
3. If key is reused elsewhere, do not delete; rename/document as needed.

## Verification Checklist

1. `rg -n "Text\\(\"|Button\\(|TextButton\\(" apps/tracer_android` to spot hardcoded button labels.
2. Build and style verify:
   - `python .\scripts\verify.py --app tracer_android --profile android_style *>&1 | Tee-Object .\scripts\docs\android-check.log`
3. Manually switch app language (`中文 / English / 日本語`) and confirm button labels render correctly.
