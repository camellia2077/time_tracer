# Android Reference: Insights Presentation

## Purpose

Describe the user-visible insights, query, and chart presentation behavior.

## When To Open

- Open this when the task changes insights mode switching, chart behavior, or insights result rendering.

## What This Doc Does Not Cover

- Core stats contracts
- Full runtime query protocol
- Broad architecture ownership

## Behavior Summary

- Insights results support text-oriented and chart-oriented presentation.
- In Text mode, the period selector exposes `Activities | Breakdown | Text`.
  Activities opens with a time-independent overview (total time, active days,
  record count, active-day average, and each parent activity's duration/share),
  and for Day, Week, Month, and Year exposes a compact calendar period row.
  The row opens a bottom sheet with a temporary selection; only `Done` applies
  it and requests the selected same-kind period. Enabling comparison reveals a
  calendar control on the next row; its bottom sheet selects the independent
  same-kind comparison period. The Activities period row continues to control
  only the current displayed content. The comparison keeps the current activity
  tree as the displayed scope and marks a zero-baseline increase as new rather
  than fabricating a percentage. It then offers a `Records` drill-down. Its
  aggregate hierarchy comes directly
  from Core's structured `project_tree`; each node expands independently and
  shows its share of the immediately containing node.
  The drill-down keeps records folded under days for a single month and under
  months then days for multi-month windows. Its rows reuse the Day timeline's
  record presentation, but use fixed-height rails to represent order rather
  than duration; Breakdown is aggregated hierarchy analysis; Text remains the
  full Markdown report.
- Chart behavior is driven by query/insights UI state, not by app-level route logic.
- Tree/insights rendering prefers structured data where available and falls back only where the product still allows it.
- The day timeline preserves Core record kinds. An `end_only` record counts as
  a timeline detail and active day, but is rendered as one localized end-time
  point without a duration value or interval line.
- The text Breakdown tree scrolls horizontally when its visible hierarchy is
  wider than the viewport. Its scrollable width follows the deepest visible
  node, so deeper hierarchies remain accessible instead of being compressed or
  clipped.
- Markdown reports retain their text-first summary and explanatory content, but
  nested activity lists can collapse or expand their child projects. This makes
  a long period report easier to browse without changing it into the structured
  Breakdown analysis view.
- Breakdown horizontal bars render every visible item at its natural row height; the Insights page owns vertical scrolling instead of placing bars in a fixed-height inner viewport.

## Regression Coverage

- Core structured-insights data-layer coverage verifies that an `end_only` detail
  survives ingest and query with an empty start time, its end time, and zero
  duration while remaining part of the activity count.
- Android runtime parser tests verify the `record_kind` mapping, and the
  feature-insights Compose test verifies the localized end-time presentation.

## Core Flow

- `feature-insights` owns query parameters, result rendering, chart rendering, and insights-side UI state.
- App composition root only injects route-level preferences and dependencies.

## First Code Entry Points

- `apps/android/feature-insights/src/main/java/com/example/tracer/ui/screen/QueryInsightsTabContent.kt`
- `apps/android/feature-insights/src/main/java/com/example/tracer/ui/screen/QueryInsightsResultDisplay.kt`
- `apps/android/feature-insights/src/main/java/com/example/tracer/ui/screen/InsightsChartResultContent.kt`
