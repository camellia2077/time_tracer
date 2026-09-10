package com.example.tracer

import com.example.tracer.ui.components.formatWeekRangeText
import java.time.LocalDate

internal fun formatInsightsActivityPeriodSummary(
    insightsMode: InsightsMode,
    draft: InsightsPeriodSelection
): String = when (insightsMode) {
    InsightsMode.DAY -> draft.date.takeIf { it.length == 8 }?.let {
        runCatching {
            LocalDate.of(
                it.take(4).toInt(),
                it.substring(4, 6).toInt(),
                it.takeLast(2).toInt()
            ).toString()
        }.getOrDefault(it)
    } ?: draft.date
    InsightsMode.WEEK -> resolveIsoWeekSelection(draft.week)?.let { selection ->
        "${formatWeekRangeText(selection.weekStart, selection.weekEnd)} · W${draft.week.takeLast(2)}"
    } ?: draft.week
    InsightsMode.MONTH -> draft.month.takeIf { it.length == 6 }?.let {
        "${it.take(4)}-${it.takeLast(2)}"
    } ?: draft.month
    InsightsMode.YEAR -> draft.year
    InsightsMode.RANGE, InsightsMode.RECENT -> ""
}
