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

## Core Flow

- `feature-report` owns query parameters, result rendering, chart rendering, and report-side UI state.
- App composition root only injects route-level preferences and dependencies.

## First Code Entry Points

- `apps/android/feature-report/src/main/java/com/example/tracer/ui/screen/QueryReportTabContent.kt`
- `apps/android/feature-report/src/main/java/com/example/tracer/ui/screen/QueryReportResultDisplay.kt`
- `apps/android/feature-report/src/main/java/com/example/tracer/ui/screen/ReportChartResultContent.kt`
