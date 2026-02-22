# tracer_android Preference Storage

This document defines where UI preference state is stored, and how it should be read/written.

## Scope

Current focus:

1. Report chart `root node` selection persistence.
2. Report chart `lookback days` persistence (optional but recommended with root).
3. Report chart `show average line` toggle persistence.

This is for UI preference state only. It is not business data storage.

## Storage Location

Use Android `DataStore<Preferences>`:

1. File owner: `apps/tracer_android/app/src/main/java/com/example/tracer/data/UserPreferencesRepository.kt`
2. DataStore declaration:
   - `val Context.dataStore ... preferencesDataStore(name = "settings")`
3. Backing file: `settings.preferences_pb` (app-private storage).

Do not store chart preference in SQLite runtime DB.

## Key Design

Add keys in `UserPreferencesRepository.PreferencesKeys`:

1. `report_chart_selected_root` (`string`)
2. `report_chart_lookback_days` (`int`)
3. `report_chart_show_average_line` (`boolean`)

Recommended defaults:

1. `selected_root = ""` (means All Roots)
2. `lookback_days = 7`
3. `show_average_line = false`

## Read Path

1. `UserPreferencesRepository` exposes a `Flow` for chart preferences.
2. `TracerScreen` collects that flow (same pattern as `recordSuggestionPreferences`).
3. Collected values are pushed into `QueryReportViewModel` on first load and when preferences change.

## Write Path

When user changes chart inputs:

1. UI event updates `QueryReportViewModel` state immediately (for responsive UI).
2. The same event writes to `UserPreferencesRepository` (DataStore edit).
3. Writes should be debounced only if needed; default immediate write is acceptable.

## Fallback Rules

When chart query response returns a root list:

1. If stored root exists in returned roots, keep it.
2. If stored root does not exist, fallback to `""` (All Roots).
3. If `lookback_days <= 0`, clamp to default `7`.

## Ownership

1. Preference schema and persistence: `app` module (`UserPreferencesRepository`).
2. Query state machine and validation: `feature-report` module (`QueryReportViewModel`).
3. Bridge wiring between repository flow and viewmodel events: `app` module (`TracerScreen`).
