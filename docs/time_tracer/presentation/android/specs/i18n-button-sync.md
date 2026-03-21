# Android i18n Resource Sync

## Purpose

Define the minimum rule for keeping visible Android strings aligned across supported locales.

## When To Open

- Open this when a task adds, removes, or changes visible text in Android UI.

## What This Doc Does Not Cover

- Full localization strategy
- Product copy decisions
- Non-Android clients

## Locale Baseline

Keep these locales aligned for active Android UI strings:

- English: `values/strings.xml`
- Chinese: `values-zh/strings.xml`
- Japanese: `values-ja/strings.xml`

## Where To Change Strings

- App-level UI strings:
  - `apps/android/app/src/main/res/values*`
- Feature-level UI strings:
  - the matching `values*` files inside the touched feature module

Do not hardcode user-visible text in Compose code.

## Change Rules

- Add:
  - add the key in all supported locales in the touched module
- Update:
  - keep the key stable if the meaning is unchanged
  - update all locales in the same change
- Remove:
  - remove usage first
  - then remove the key from all matching locales

## Quick Verification

- Search for suspicious hardcoded UI text in the touched module.
- Run:

```powershell
python tools/run.py verify --app tracer_android --profile android_style --concise
```
