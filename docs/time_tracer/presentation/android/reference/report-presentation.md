# Android Reference: Report Presentation

## Purpose

Describe the user-visible report, query, and chart presentation behavior.

## When To Open

- Open this when the task changes report mode switching, chart behavior, or report result rendering.

## What This Doc Does Not Cover

- Core stats contracts
- Full runtime query protocol
- Broad architecture ownership

## Behavior Summary

- Report results support text-oriented and chart-oriented presentation.
- Chart behavior is driven by query/report UI state, not by app-level route logic.
- Tree/report rendering prefers structured data where available and falls back only where the product still allows it.
- The day timeline preserves Core record kinds. An `end_only` record counts as
  a timeline detail and active day, but is rendered as one localized end-time
  point without a duration value or interval line.
- Breakdown horizontal bars render every visible item at its natural row height; the Report page owns vertical scrolling instead of placing bars in a fixed-height inner viewport.

## Regression Coverage

- Core structured-report data-layer coverage verifies that an `end_only` detail
  survives ingest and query with an empty start time, its end time, and zero
  duration while remaining part of the activity count.
- Android runtime parser tests verify the `record_kind` mapping, and the
  feature-report Compose test verifies the localized end-time presentation.

## Core Flow

- `feature-report` owns query parameters, result rendering, chart rendering, and report-side UI state.
- App composition root only injects route-level preferences and dependencies.

## First Code Entry Points

- `apps/android/feature-report/src/main/java/com/example/tracer/ui/screen/QueryReportTabContent.kt`
- `apps/android/feature-report/src/main/java/com/example/tracer/ui/screen/QueryReportResultDisplay.kt`
- `apps/android/feature-report/src/main/java/com/example/tracer/ui/screen/ReportChartResultContent.kt`
