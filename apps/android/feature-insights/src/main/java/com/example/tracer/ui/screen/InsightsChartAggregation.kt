package com.example.tracer

import java.time.LocalDate
import java.time.YearMonth
import java.time.temporal.ChronoUnit

internal fun shouldAggregateChartPointsByMonth(
    insightsMode: InsightsMode,
    fromDateIso: String?,
    toDateIso: String?
): Boolean {
    if (insightsMode == InsightsMode.YEAR) return true
    if (insightsMode != InsightsMode.RANGE) return false
    val from = fromDateIso?.let { runCatching { LocalDate.parse(it) }.getOrNull() } ?: return false
    val to = toDateIso?.let { runCatching { LocalDate.parse(it) }.getOrNull() } ?: return false
    return !to.isBefore(from) && ChronoUnit.DAYS.between(from, to) + 1 > 90
}

/** Builds the Year chart series as one total-duration point per calendar month. */
internal fun aggregateYearChartPoints(
    points: List<InsightsChartPoint>,
    fromDateIso: String?,
    toDateIso: String?
): List<InsightsChartPoint> {
    val parsedPoints = points.mapNotNull { point ->
        val date = runCatching { LocalDate.parse(point.date) }.getOrNull() ?: return@mapNotNull null
        date to point.durationSeconds.coerceAtLeast(0L)
    }
    val from = fromDateIso?.let { runCatching { LocalDate.parse(it) }.getOrNull() }
    val to = toDateIso?.let { runCatching { LocalDate.parse(it) }.getOrNull() }
    val firstMonth = from?.let { YearMonth.of(it.year, it.month) }
        ?: parsedPoints.minOfOrNull { YearMonth.of(it.first.year, it.first.month) }
    val lastMonth = to?.let { YearMonth.of(it.year, it.month) }
        ?: parsedPoints.maxOfOrNull { YearMonth.of(it.first.year, it.first.month) }
    val first = firstMonth ?: return emptyList()
    val last = lastMonth ?: return emptyList()
    if (last.isBefore(first)) return emptyList()

    val totalsByMonth = parsedPoints.groupingBy { YearMonth.of(it.first.year, it.first.month) }
        .fold(0L) { total, (_, duration) -> total + duration }
    val result = mutableListOf<InsightsChartPoint>()
    var month = first
    while (!month.isAfter(last)) {
        val date = month.atDay(1)
        result += InsightsChartPoint(
            date = date.toString(),
            durationSeconds = totalsByMonth[month] ?: 0L,
            epochDay = date.toEpochDay()
        )
        month = month.plusMonths(1)
    }
    return result
}
