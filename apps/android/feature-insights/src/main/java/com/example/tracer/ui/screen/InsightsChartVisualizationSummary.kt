package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import com.example.tracer.feature.insights.R
import java.time.LocalDate
import java.time.temporal.ChronoUnit
import java.util.Locale
import kotlin.math.roundToLong

@Composable
internal fun InsightsChartVisualizationSummary(
    sortedChartPoints: List<InsightsChartPoint>,
    recordedPoints: List<InsightsChartPoint>,
    selectedPointIndex: Int,
    chartFromDateIso: String?,
    chartToDateIso: String?,
    chartAverageDurationSeconds: Long,
    chartTotalOccurrenceCount: Long,
    chartTotalDurationSeconds: Long,
    chartAverageDurationPerOccurrenceSeconds: Long,
    chartModeDurationSeconds: Double?,
    chartMedianDurationSeconds: Double?,
    chartMinimumDurationSeconds: Double?,
    chartMaximumDurationSeconds: Double?,
    chartLowerQuartileDurationSeconds: Double?,
    chartUpperQuartileDurationSeconds: Double?,
    chartCoefficientOfVariation: Double?,
    chartMeanAbsoluteDeviationSeconds: Double?,
    chartVisualMode: InsightsChartVisualMode
) {
    val selectedPoint = sortedChartPoints.getOrNull(selectedPointIndex)
    if (selectedPoint != null) {
        Text(
            text = stringResource(
                R.string.insights_chart_selected_detail,
                selectedPoint.date,
                formatDurationHoursMinutes(selectedPoint.durationSeconds)
            ),
            style = MaterialTheme.typography.bodySmall,
            color = MaterialTheme.colorScheme.primary
        )
    }

    val recordedDays = recordedPoints
        .mapNotNull { point -> runCatching { LocalDate.parse(point.date) }.getOrNull() }
        .distinct()
        .size
    val requestedDays = runCatching {
        val from = chartFromDateIso?.let(LocalDate::parse)
        val to = chartToDateIso?.let(LocalDate::parse)
        if (from == null || to == null || to.isBefore(from)) {
            null
        } else {
            ChronoUnit.DAYS.between(from, to).toInt() + 1
        }
    }.getOrNull()
    val totalDays = requestedDays ?: recordedDays
    if (totalDays > 0) {
        Text(
            text = stringResource(
                R.string.insights_chart_record_completeness,
                recordedDays,
                totalDays,
                (recordedDays * 100.0 / totalDays).toInt()
            ),
            style = MaterialTheme.typography.bodySmall,
            color = MaterialTheme.colorScheme.primary
        )
    }

    Text(
        text = stringResource(
            R.string.insights_chart_total_occurrences,
            chartTotalOccurrenceCount.coerceAtLeast(0L)
        ),
        style = MaterialTheme.typography.bodySmall,
        color = MaterialTheme.colorScheme.primary
    )
    Text(
        text = stringResource(
            R.string.insights_chart_total_duration,
            formatDurationHoursMinutes(chartTotalDurationSeconds.coerceAtLeast(0L))
        ),
        style = MaterialTheme.typography.bodySmall,
        color = MaterialTheme.colorScheme.primary
    )

    val averageDurationSeconds = chartAverageDurationSeconds.coerceAtLeast(0L)
    Text(
        text = stringResource(
            R.string.insights_chart_average_duration,
            formatDurationHoursMinutes(averageDurationSeconds)
        ),
        style = MaterialTheme.typography.bodySmall,
        color = MaterialTheme.colorScheme.primary
    )
    Text(
        text = stringResource(
            R.string.insights_chart_average_per_occurrence,
            formatDurationHoursMinutes(
                chartAverageDurationPerOccurrenceSeconds.coerceAtLeast(0L)
            )
        ),
        style = MaterialTheme.typography.bodySmall,
        color = MaterialTheme.colorScheme.primary
    )
    chartModeDurationSeconds?.let { durationSeconds ->
        Text(
            text = stringResource(
                R.string.insights_chart_mode_duration,
                formatDurationHoursMinutes(durationSeconds.roundToLong())
            ),
            style = MaterialTheme.typography.bodySmall,
            color = MaterialTheme.colorScheme.primary
        )
    }
    chartMedianDurationSeconds?.let { durationSeconds ->
        Text(
            text = stringResource(
                R.string.insights_chart_median_duration,
                formatDurationHoursMinutes(durationSeconds.roundToLong())
            ),
            style = MaterialTheme.typography.bodySmall,
            color = MaterialTheme.colorScheme.primary
        )
    }
    if (chartMinimumDurationSeconds != null && chartMaximumDurationSeconds != null) {
        Text(
            text = stringResource(
                R.string.insights_chart_full_duration_range,
                formatDurationHoursMinutes(chartMinimumDurationSeconds.roundToLong()),
                formatDurationHoursMinutes(chartMaximumDurationSeconds.roundToLong())
            ),
            style = MaterialTheme.typography.bodySmall,
            color = MaterialTheme.colorScheme.primary
        )
    }
    if (chartLowerQuartileDurationSeconds != null && chartUpperQuartileDurationSeconds != null) {
        Text(
            text = stringResource(
                R.string.insights_chart_typical_daily_range,
                formatDurationHoursMinutes(chartLowerQuartileDurationSeconds.roundToLong()),
                formatDurationHoursMinutes(chartUpperQuartileDurationSeconds.roundToLong())
            ),
            style = MaterialTheme.typography.bodySmall,
            color = MaterialTheme.colorScheme.primary
        )
    }
    chartCoefficientOfVariation?.let { coefficient ->
        Text(
            text = stringResource(
                R.string.insights_chart_coefficient_of_variation,
                String.format(Locale.ROOT, "%.0f%%", coefficient * 100.0)
            ),
            style = MaterialTheme.typography.bodySmall,
            color = MaterialTheme.colorScheme.primary
        )
    }
    chartMeanAbsoluteDeviationSeconds?.let { durationSeconds ->
        Text(
            text = stringResource(
                R.string.insights_chart_mean_absolute_deviation,
                formatDurationHoursMinutes(durationSeconds.roundToLong())
            ),
            style = MaterialTheme.typography.bodySmall,
            color = MaterialTheme.colorScheme.primary
        )
    }
    if (chartVisualMode == InsightsChartVisualMode.LINE ||
        chartVisualMode == InsightsChartVisualMode.BAR
    ) {
        val start = sortedChartPoints.firstOrNull()?.date?.toMonthDayLabel().orEmpty()
        val middle =
            sortedChartPoints.getOrNull(sortedChartPoints.size / 2)?.date?.toMonthDayLabel().orEmpty()
        val end = sortedChartPoints.lastOrNull()?.date?.toMonthDayLabel().orEmpty()
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.SpaceBetween,
            verticalAlignment = Alignment.CenterVertically
        ) {
            Text(text = start, style = MaterialTheme.typography.labelSmall)
            Text(text = middle, style = MaterialTheme.typography.labelSmall)
            Text(text = end, style = MaterialTheme.typography.labelSmall)
        }
    }
}
