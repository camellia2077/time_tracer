# Android Preference Storage

## Purpose

Describe where Android UI preference state lives and which layer owns read/write behavior.

## When To Open

- Open this when the task adds or changes persisted UI preferences.

## What This Doc Does Not Cover

- Runtime business data storage
- SQLite schema
- Feature behavior details

## Storage Owner

Android UI preferences use `DataStore<Preferences>`:

- Repository owner:
  - `apps/android/app/src/main/java/com/example/tracer/data/UserPreferencesRepository.kt`
- Backing store:
  - app-private `settings.preferences_pb`

Do not store UI preferences in the runtime SQLite database.

## Current Preference Groups

- Theme and appearance:
  - theme mode
  - dark theme style
  - theme palette
  - the selected UI and Report color systems are defined in `../ui/color-system.md` and `../ui/report/README.md`; the selected palette is user-persisted
- App language
- Record assistance preferences:
  - suggestion lookback days
  - suggestion top-N
  - quick activities
  - assist panel expansion flags
- Report chart preference:
  - show average line
  - preferred chart semantic mode (Breakdown or Trend)
  - preferred Trend top-level activity

## Ownership

- Preference schema and persistence:
  - `app` module
- Theme and language write path:
  - `ThemeViewModel`
- Record/report preference bridge into feature state:
  - `TracerScreen` and its route helpers
- Feature modules consume injected state/callbacks; they should not depend on `DataStore` directly.

## Read / Write Rule

- Read preferences through repository flows.
- Write preferences through app-layer callbacks or app-layer view models.
- Keep defaults and normalization in `UserPreferencesRepository`.
