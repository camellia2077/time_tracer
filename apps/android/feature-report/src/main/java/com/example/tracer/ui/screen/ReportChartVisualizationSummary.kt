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
import com.example.tracer.feature.report.R

@Composable
internal fun ReportChartVisualizationSummary(
    sortedChartPoints: List<ReportChartPoint>,
    selectedPointIndex: Int,
    chartAverageDurationSeconds: Long?,
    chartUsesLegacyStatsFallback: Boolean,
    chartVisualMode: ReportChartVisualMode
) {
    val averageDurationSeconds = chartAverageDurationSeconds?.coerceAtLeast(0L) ?: 0L
    Text(
        text = stringResource(
            R.string.report_chart_average_duration,
            formatDurationHoursMinutes(averageDurationSeconds)
        ),
        style = MaterialTheme.typography.bodySmall,
        color = MaterialTheme.colorScheme.primary
    )
    if (chartUsesLegacyStatsFallback) {
        Text(
            text = stringResource(R.string.report_chart_legacy_fallback_notice),
            style = MaterialTheme.typography.bodySmall,
            color = MaterialTheme.colorScheme.secondary
        )
    }

    val selectedPoint = sortedChartPoints.getOrNull(selectedPointIndex)
    if (selectedPoint != null) {
        Text(
            text = stringResource(
                R.string.report_chart_selected_detail,
                selectedPoint.date,
                formatDurationHoursMinutes(selectedPoint.durationSeconds)
            ),
            style = MaterialTheme.typography.bodySmall,
            color = MaterialTheme.colorScheme.primary
        )
    }

    if (chartVisualMode == ReportChartVisualMode.LINE ||
        chartVisualMode == ReportChartVisualMode.BAR
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
