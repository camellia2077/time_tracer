package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.aspectRatio
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.remember
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.report.R
import java.time.LocalDate
import java.time.YearMonth
import java.time.format.DateTimeFormatter
import java.util.Locale

@Composable
internal fun ReportMultiMonthHeatmap(
    points: List<ReportChartPoint>,
    selectedIndex: Int,
    reportMode: ReportMode,
    heatmapTomlConfig: ReportHeatmapTomlConfig,
    heatmapStylePreference: ReportHeatmapStylePreference,
    isAppDarkThemeActive: Boolean,
    onPointSelected: (Int) -> Unit,
    modifier: Modifier = Modifier
) {
    val months = remember(points, reportMode) {
        resolveMultiMonthHeatmapMonths(points, reportMode)
    }
    val formatter = remember { DateTimeFormatter.ofPattern("MMM", Locale.getDefault()) }

    Column(
        modifier = modifier,
        verticalArrangement = Arrangement.spacedBy(12.dp)
    ) {
        Text(
            text = stringResource(R.string.report_chart_heatmap_multi_month_hint),
            style = MaterialTheme.typography.labelSmall,
            color = MaterialTheme.colorScheme.onSurfaceVariant
        )
        months.chunked(2).forEach { monthRow ->
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.spacedBy(12.dp)
            ) {
                monthRow.forEach { month ->
                    MonthHeatmapCard(
                        month = month,
                        points = points,
                        selectedIndex = selectedIndex,
                        heatmapTomlConfig = heatmapTomlConfig,
                        heatmapStylePreference = heatmapStylePreference,
                        isAppDarkThemeActive = isAppDarkThemeActive,
                        onPointSelected = onPointSelected,
                        title = formatter.format(month),
                        modifier = Modifier.weight(1f)
                    )
                }
                if (monthRow.size == 1) {
                    Column(modifier = Modifier.weight(1f)) {}
                }
            }
        }
    }
}

@Composable
private fun MonthHeatmapCard(
    month: YearMonth,
    points: List<ReportChartPoint>,
    selectedIndex: Int,
    heatmapTomlConfig: ReportHeatmapTomlConfig,
    heatmapStylePreference: ReportHeatmapStylePreference,
    isAppDarkThemeActive: Boolean,
    onPointSelected: (Int) -> Unit,
    title: String,
    modifier: Modifier = Modifier
) {
    Column(modifier = modifier, verticalArrangement = Arrangement.spacedBy(4.dp)) {
        Text(
            text = title,
            style = MaterialTheme.typography.labelSmall,
            color = MaterialTheme.colorScheme.onSurfaceVariant
        )
        ReportHeatmapChart(
            points = points,
            selectedIndex = selectedIndex,
            mode = ReportHeatmapMode.MONTH,
            anchorDateOverride = month.atDay(1),
            heatmapTomlConfig = heatmapTomlConfig,
            heatmapStylePreference = heatmapStylePreference,
            isAppDarkThemeActive = isAppDarkThemeActive,
            onPointSelected = onPointSelected,
            modifier = Modifier
                .fillMaxWidth()
                .aspectRatio(1f)
        )
    }
}

internal fun resolveMultiMonthHeatmapMonths(
    points: List<ReportChartPoint>,
    reportMode: ReportMode
): List<YearMonth> {
    val pointMonths = points.mapNotNull { point ->
        runCatching { YearMonth.from(LocalDate.parse(point.date)) }.getOrNull()
    }.distinct().sorted()
    if (pointMonths.isEmpty()) {
        return emptyList()
    }
    if (reportMode != ReportMode.YEAR) {
        return pointMonths
    }

    val year = pointMonths.last().year
    return (1..12).map { month -> YearMonth.of(year, month) }
}
