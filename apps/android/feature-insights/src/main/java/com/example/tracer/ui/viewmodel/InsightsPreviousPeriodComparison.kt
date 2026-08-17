package com.example.tracer

import java.time.LocalDate
import java.time.YearMonth
import java.time.format.DateTimeFormatter

internal data class ComparisonPeriodRequest(
    val request: TemporalInsightsQueryRequest,
    val label: String
)

internal fun resolveDefaultComparisonPeriodRequest(
    state: QueryInsightsUiState,
    locale: String
): ComparisonPeriodRequest? = when (state.insightsMode) {
    InsightsMode.RANGE, InsightsMode.RECENT -> resolveAutomaticPreviousPeriodRequest(state, locale)
    else -> defaultComparisonPeriodDraft(state)?.let { draft ->
        resolveComparisonPeriodRequest(state, draft, locale)
    }
}

/** Resolves a comparison window for Trend without requiring the Activities insights payload. */
internal fun resolveDefaultChartComparisonPeriodRequest(
    state: QueryInsightsUiState,
    locale: String
): ComparisonPeriodRequest? = when (state.insightsMode) {
    InsightsMode.DAY -> state.insightsDate.toBasicIsoDate()?.minusDays(1)?.let { day ->
        previousRequest(
            displayMode = InsightsDisplayMode.DAY,
            selection = TemporalSelectionPayload(
                kind = TemporalSelectionKind.SINGLE_DAY,
                date = day.toString()
            ),
            start = day,
            end = day,
            locale = locale
        )
    }

    InsightsMode.WEEK -> resolveIsoWeekSelection(state.insightsWeek)?.let { week ->
        val previousStart = week.weekStart.minusDays(7)
        val previousEnd = week.weekEnd.minusDays(7)
        previousRequest(
            displayMode = InsightsDisplayMode.WEEK,
            selection = TemporalSelectionPayload(
                kind = TemporalSelectionKind.DATE_RANGE,
                startDate = previousStart.toString(),
                endDate = previousEnd.toString()
            ),
            start = previousStart,
            end = previousEnd,
            locale = locale
        )
    }

    InsightsMode.MONTH -> state.insightsMonth.toInsightsYearMonth()?.minusMonths(1)?.let { month ->
        previousRequest(
            displayMode = InsightsDisplayMode.MONTH,
            selection = TemporalSelectionPayload(
                kind = TemporalSelectionKind.DATE_RANGE,
                startDate = month.atDay(1).toString(),
                endDate = month.atEndOfMonth().toString()
            ),
            start = month.atDay(1),
            end = month.atEndOfMonth(),
            locale = locale
        )
    }

    InsightsMode.YEAR -> null

    InsightsMode.RANGE -> {
        val start = state.insightsRangeStartDate.toBasicIsoDate()
        val end = state.insightsRangeEndDate.toBasicIsoDate()
        if (start == null || end == null || end.isBefore(start)) null else {
            val days = java.time.temporal.ChronoUnit.DAYS.between(start, end) + 1
            val previousEnd = start.minusDays(1)
            val previousStart = previousEnd.minusDays(days - 1)
            previousRequest(
                displayMode = InsightsDisplayMode.RANGE,
                selection = TemporalSelectionPayload(
                    kind = TemporalSelectionKind.DATE_RANGE,
                    startDate = previousStart.toString(),
                    endDate = previousEnd.toString()
                ),
                start = previousStart,
                end = previousEnd,
                locale = locale
            )
        }
    }

    InsightsMode.RECENT -> {
        val days = state.insightsRecentDays.toIntOrNull()?.takeIf { it > 0 }
        val currentAnchor = state.insightsDate.toBasicIsoDate()
        if (days == null || currentAnchor == null) null else {
            val previousEnd = currentAnchor.minusDays(days.toLong())
            val previousStart = previousEnd.minusDays(days.toLong() - 1)
            previousRequest(
                displayMode = InsightsDisplayMode.RECENT,
                selection = TemporalSelectionPayload(
                    kind = TemporalSelectionKind.RECENT_DAYS,
                    days = days,
                    anchorDate = previousEnd.toString()
                ),
                start = previousStart,
                end = previousEnd,
                locale = locale
            )
        }
    }
}

internal fun resolveChartComparisonPeriodRequest(
    state: QueryInsightsUiState,
    selection: InsightsPeriodSelection,
    locale: String
): ComparisonPeriodRequest? = when (state.insightsMode) {
    InsightsMode.DAY -> selection.date.toBasicIsoDate()?.let { day ->
        previousRequest(
            displayMode = InsightsDisplayMode.DAY,
            selection = TemporalSelectionPayload(
                kind = TemporalSelectionKind.SINGLE_DAY,
                date = day.toString()
            ),
            start = day,
            end = day,
            locale = locale
        )
    }

    InsightsMode.WEEK -> resolveIsoWeekSelection(selection.week)?.let { week ->
        previousRequest(
            displayMode = InsightsDisplayMode.WEEK,
            selection = TemporalSelectionPayload(
                kind = TemporalSelectionKind.DATE_RANGE,
                startDate = week.weekStart.toString(),
                endDate = week.weekEnd.toString()
            ),
            start = week.weekStart,
            end = week.weekEnd,
            locale = locale
        )
    }

    InsightsMode.MONTH -> selection.month.toIsoYearMonth()?.let { month ->
        previousRequest(
            displayMode = InsightsDisplayMode.MONTH,
            selection = TemporalSelectionPayload(
                kind = TemporalSelectionKind.DATE_RANGE,
                startDate = month.atDay(1).toString(),
                endDate = month.atEndOfMonth().toString()
            ),
            start = month.atDay(1),
            end = month.atEndOfMonth(),
            locale = locale
        )
    }

    InsightsMode.YEAR -> null

    InsightsMode.RANGE, InsightsMode.RECENT -> null
}

internal fun defaultComparisonPeriodDraft(
    state: QueryInsightsUiState
): InsightsPeriodSelection? {
    val period = state.insightsMode.toDataTreePeriod()
    if (state.insightsResultsByPeriod[period] == null) return null
    return when (state.insightsMode) {
        InsightsMode.DAY -> state.insightsDate.toBasicIsoDate()?.minusDays(1)?.let { day ->
            state.toPeriodSelection().copy(date = day.format(BASIC_DATE_FORMATTER))
        }

        InsightsMode.WEEK -> resolveIsoWeekSelection(state.insightsWeek)?.let { week ->
            state.toPeriodSelection().copy(
                week = formatIsoWeekDigits(week.weekStart.minusDays(7))
            )
        }

        InsightsMode.MONTH -> state.insightsMonth.toInsightsYearMonth()?.minusMonths(1)?.let { month ->
            state.toPeriodSelection().copy(month = month.toString())
        }

        InsightsMode.YEAR -> state.insightsYear.toIntOrNull()?.let { year ->
            state.toPeriodSelection().copy(year = (year - 1).toString())
        }

        // These modes retain the existing adjacent-period comparison behavior because their
        // Activities parameter card does not expose the compact calendar picker.
        InsightsMode.RANGE, InsightsMode.RECENT -> null
    }
}

internal fun resolveComparisonPeriodRequest(
    state: QueryInsightsUiState,
    selection: InsightsPeriodSelection,
    locale: String
): ComparisonPeriodRequest? {
    val period = state.insightsMode.toDataTreePeriod()
    if (state.insightsResultsByPeriod[period] == null) return null
    return when (state.insightsMode) {
        InsightsMode.DAY -> selection.date.toBasicIsoDate()?.let { day ->
            previousRequest(
                displayMode = InsightsDisplayMode.DAY,
                selection = TemporalSelectionPayload(
                    kind = TemporalSelectionKind.SINGLE_DAY,
                    date = day.toString()
                ),
                start = day,
                end = day,
                locale = locale
            )
        }

        InsightsMode.WEEK -> resolveIsoWeekSelection(selection.week)?.let { week ->
            previousRequest(
                displayMode = InsightsDisplayMode.WEEK,
                selection = TemporalSelectionPayload(
                    kind = TemporalSelectionKind.DATE_RANGE,
                    startDate = week.weekStart.toString(),
                    endDate = week.weekEnd.toString()
                ),
                start = week.weekStart,
                end = week.weekEnd,
                locale = locale
            )
        }

        InsightsMode.MONTH -> selection.month.toIsoYearMonth()?.let { month ->
            previousRequest(
                displayMode = InsightsDisplayMode.MONTH,
                selection = TemporalSelectionPayload(
                    kind = TemporalSelectionKind.DATE_RANGE,
                    startDate = month.atDay(1).toString(),
                    endDate = month.atEndOfMonth().toString()
                ),
                start = month.atDay(1),
                end = month.atEndOfMonth(),
                locale = locale
            )
        }

        InsightsMode.YEAR -> selection.year.toIntOrNull()?.let { year ->
            previousRequest(
                displayMode = InsightsDisplayMode.YEAR,
                selection = TemporalSelectionPayload(
                    kind = TemporalSelectionKind.DATE_RANGE,
                    startDate = "$year-01-01",
                    endDate = "$year-12-31"
                ),
                start = LocalDate.of(year, 1, 1),
                end = LocalDate.of(year, 12, 31),
                locale = locale
            )
        }

        InsightsMode.RANGE, InsightsMode.RECENT -> null
    }
}

private fun resolveAutomaticPreviousPeriodRequest(
    state: QueryInsightsUiState,
    locale: String
): ComparisonPeriodRequest? = when (state.insightsMode) {
    InsightsMode.RANGE -> {
        val start = state.insightsRangeStartDate.toBasicIsoDate()
        val end = state.insightsRangeEndDate.toBasicIsoDate()
        if (start == null || end == null || end.isBefore(start)) null else {
            val days = java.time.temporal.ChronoUnit.DAYS.between(start, end) + 1
            val previousEnd = start.minusDays(1)
            val previousStart = previousEnd.minusDays(days - 1)
            previousRequest(
                displayMode = InsightsDisplayMode.RANGE,
                selection = TemporalSelectionPayload(
                    kind = TemporalSelectionKind.DATE_RANGE,
                    startDate = previousStart.toString(),
                    endDate = previousEnd.toString()
                ),
                start = previousStart,
                end = previousEnd,
                locale = locale
            )
        }
    }

    InsightsMode.RECENT -> {
        val metadata = (state.insightsSummariesByPeriod[DataTreePeriod.RECENT]
            as? InsightsSummary.WindowMetadata)?.metadata
        val days = metadata?.requestedDays ?: return null
        val previousAnchor = metadata.endDate.toIsoDate()?.minusDays(days.toLong()) ?: return null
        val previousStart = previousAnchor.minusDays(days.toLong() - 1)
        previousRequest(
            displayMode = InsightsDisplayMode.RECENT,
            selection = TemporalSelectionPayload(
                kind = TemporalSelectionKind.RECENT_DAYS,
                days = days,
                anchorDate = previousAnchor.toString()
            ),
            start = previousStart,
            end = previousAnchor,
            locale = locale
        )
    }

    else -> null
}

internal fun QueryInsightsUiState.toPeriodSelection(): InsightsPeriodSelection =
    InsightsPeriodSelection(
        date = insightsDate,
        month = insightsMonth,
        year = insightsYear,
        week = insightsWeek
    )

internal fun QueryInsightsUiState.clearPeriodComparison(): QueryInsightsUiState = copy(
    periodComparison = InsightsPeriodComparisonState.Hidden,
    periodComparisonVersion = periodComparisonVersion + 1
)

internal fun QueryInsightsUiState.clearTrendChartComparison(): QueryInsightsUiState = copy(
    trendChartComparison = InsightsPeriodComparisonState.Hidden,
    trendChartComparisonVersion = trendChartComparisonVersion + 1
)

private fun previousRequest(
    displayMode: InsightsDisplayMode,
    selection: TemporalSelectionPayload,
    start: LocalDate,
    end: LocalDate,
    locale: String
): ComparisonPeriodRequest = ComparisonPeriodRequest(
    request = TemporalInsightsQueryRequest(
        displayMode = displayMode,
        selection = selection,
        locale = locale
    ),
    label = if (start == end) start.toString() else "$start – $end"
)

private fun String.toBasicIsoDate(): LocalDate? = runCatching {
    LocalDate.parse(trim(), DateTimeFormatter.BASIC_ISO_DATE)
}.getOrNull()

private fun String.toIsoDate(): LocalDate? = runCatching {
    LocalDate.parse(trim())
}.getOrNull()

private fun String.toInsightsYearMonth(): YearMonth? = runCatching {
    YearMonth.parse(trim(), DateTimeFormatter.ofPattern("yyyyMM"))
}.getOrNull()

private fun String.toIsoYearMonth(): YearMonth? = runCatching {
    YearMonth.parse(trim())
}.getOrNull()

private fun formatIsoWeekDigits(date: LocalDate): String {
    val fields = java.time.temporal.WeekFields.ISO
    return "%04d%02d".format(
        java.util.Locale.US,
        date.get(fields.weekBasedYear()),
        date.get(fields.weekOfWeekBasedYear())
    )
}

private val BASIC_DATE_FORMATTER: DateTimeFormatter = DateTimeFormatter.BASIC_ISO_DATE
