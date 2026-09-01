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
  - the selected UI and Insights color systems are defined in `../ui/color-system.md` and `../ui/insights/README.md`; the selected palette is user-persisted
- App language and motion:
  - selected page transition (no animation, quick fade, or light horizontal slide)
  - time display mode (24-hour or 12-hour AM/PM)
- Settings-page expansion state:
  - top-level cards
  - Theme Palette, Insights chart style, and Activity comparison subsections
- Record assistance preferences:
  - frequent activity lookback days
  - frequent activity top-N
  - last selected `Tree | Frequent | Categories` source in the activity browser
  - quick activities
  - assist panel expansion flags
- Insights chart preference:
  - show average line
  - preferred chart semantic mode (Breakdown or Trend)
  - preferred Trend top-level activity
  - pie and heatmap palette selections
  - period comparison color scheme
  - period comparison indicator style
  - daily status definitions

Android does not read, write, import, or export `config/user/charts.toml`,
`config/user/heatmap.toml`, or `config/user/insights.toml`. Their values are
not migrated.

## Ownership

- Preference schema and persistence:
  - `app` module
- Theme and language write path:
  - `ThemeViewModel`
- Record/insights preference bridge into feature state:
  - `TracerScreen` and its route helpers
- Feature modules consume injected state/callbacks; they should not depend on `DataStore` directly.

## Read / Write Rule

- Read preferences through repository flows.
- Write preferences through app-layer callbacks or app-layer view models.
- Keep defaults and normalization in `UserPreferencesRepository`.
