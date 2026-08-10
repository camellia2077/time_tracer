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
- Chart behavior is driven by query/insights UI state, not by app-level route logic.
- Tree/insights rendering prefers structured data where available and falls back only where the product still allows it.
- The day timeline preserves Core record kinds. An `end_only` record counts as
  a timeline detail and active day, but is rendered as one localized end-time
  point without a duration value or interval line.
- The text Breakdown tree scrolls horizontally when its visible hierarchy is
  wider than the viewport. Its scrollable width follows the deepest visible
  node, so deeper hierarchies remain accessible instead of being compressed or
  clipped.
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
