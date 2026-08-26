package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.BoxWithConstraints
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.aspectRatio
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.horizontalScroll
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Close
import androidx.compose.material.icons.filled.Fullscreen
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.remember
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.insights.R
import java.time.LocalDate
import java.time.YearMonth
import java.time.format.DateTimeFormatter
import java.util.Locale

@Composable
internal fun InsightsMultiMonthHeatmap(
    points: List<InsightsChartPoint>,
    selectedIndex: Int,
    insightsMode: InsightsMode,
    heatmapTomlConfig: InsightsHeatmapTomlConfig,
    heatmapStylePreference: InsightsHeatmapStylePreference,
    isAppDarkThemeActive: Boolean,
    onPointSelected: (Int) -> Unit,
    modifier: Modifier = Modifier
) {
    val months = remember(points, insightsMode) {
        resolveMultiMonthHeatmapMonths(points, insightsMode)
    }
    val formatter = remember { DateTimeFormatter.ofPattern("MMM", Locale.getDefault()) }
    var isFullscreen by remember { mutableStateOf(false) }

    Column(modifier = modifier, verticalArrangement = Arrangement.spacedBy(12.dp)) {
        Row(
            modifier = Modifier.fillMaxWidth(),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Text(
                text = stringResource(R.string.insights_chart_heatmap_multi_month_hint),
                style = MaterialTheme.typography.labelSmall,
                color = MaterialTheme.colorScheme.onSurfaceVariant,
                modifier = Modifier.weight(1f)
            )
            if (insightsMode == InsightsMode.YEAR) {
                IconButton(onClick = { isFullscreen = true }) {
                    Icon(
                        imageVector = Icons.Default.Fullscreen,
                        contentDescription = stringResource(R.string.insights_cd_expand)
                    )
                }
            }
        }

        YearHeatmapMonthGrid(
            months = months,
            formatter = formatter,
            points = points,
            selectedIndex = selectedIndex,
            heatmapTomlConfig = heatmapTomlConfig,
            heatmapStylePreference = heatmapStylePreference,
            isAppDarkThemeActive = isAppDarkThemeActive,
            onPointSelected = onPointSelected
        )
    }

    if (isFullscreen) {
        FullscreenPage(onDismissRequest = { isFullscreen = false }) {
            Column(
                modifier = Modifier
                    .fillMaxSize()
                    .padding(16.dp),
                verticalArrangement = Arrangement.spacedBy(12.dp)
            ) {
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Text(
                        text = stringResource(R.string.insights_chart_heatmap_multi_month_hint),
                        style = MaterialTheme.typography.titleMedium,
                        modifier = Modifier.weight(1f)
                    )
                    IconButton(onClick = { isFullscreen = false }) {
                        Icon(
                            imageVector = Icons.Default.Close,
                            contentDescription = stringResource(R.string.insights_cd_collapse)
                        )
                    }
                }
                YearHeatmapMonthGrid(
                    months = months,
                    formatter = formatter,
                    points = points,
                    selectedIndex = selectedIndex,
                    heatmapTomlConfig = heatmapTomlConfig,
                    heatmapStylePreference = heatmapStylePreference,
                    isAppDarkThemeActive = isAppDarkThemeActive,
                    onPointSelected = onPointSelected,
                    horizontalLandscape = true,
                    modifier = Modifier.weight(1f)
                )
            }
        }
    }
}

@Composable
private fun YearHeatmapMonthGrid(
    months: List<YearMonth>,
    formatter: DateTimeFormatter,
    points: List<InsightsChartPoint>,
    selectedIndex: Int,
    heatmapTomlConfig: InsightsHeatmapTomlConfig,
    heatmapStylePreference: InsightsHeatmapStylePreference,
    isAppDarkThemeActive: Boolean,
    onPointSelected: (Int) -> Unit,
    horizontalLandscape: Boolean = false,
    modifier: Modifier = Modifier
) {
    BoxWithConstraints(modifier = modifier) {
        val useHorizontalLandscape = horizontalLandscape && maxWidth > maxHeight
        val columns = when {
            maxWidth >= 840.dp -> 6
            maxWidth >= 500.dp -> 4
            else -> 2
        }

        if (useHorizontalLandscape) {
            val cardWidth = (maxHeight - 72.dp).coerceIn(160.dp, 260.dp)
            Row(
                modifier = Modifier.horizontalScroll(rememberScrollState()),
                horizontalArrangement = Arrangement.spacedBy(12.dp)
            ) {
                months.forEach { month ->
                    MonthHeatmapCard(
                        month = month,
                        points = points,
                        selectedIndex = selectedIndex,
                        heatmapTomlConfig = heatmapTomlConfig,
                        heatmapStylePreference = heatmapStylePreference,
                        isAppDarkThemeActive = isAppDarkThemeActive,
                        onPointSelected = onPointSelected,
                        title = formatter.format(month),
                        modifier = Modifier.width(cardWidth)
                    )
                }
            }
        } else {
            val gridModifier = if (horizontalLandscape) {
                Modifier.verticalScroll(rememberScrollState())
            } else {
                Modifier
            }
            Column(
                modifier = gridModifier,
                verticalArrangement = Arrangement.spacedBy(12.dp)
            ) {
                months.chunked(columns).forEach { monthRow ->
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
                        repeat(columns - monthRow.size) {
                            Column(modifier = Modifier.weight(1f)) {}
                        }
                    }
                }
            }
        }
    }
}

@Composable
private fun MonthHeatmapCard(
    month: YearMonth,
    points: List<InsightsChartPoint>,
    selectedIndex: Int,
    heatmapTomlConfig: InsightsHeatmapTomlConfig,
    heatmapStylePreference: InsightsHeatmapStylePreference,
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
        InsightsHeatmapChart(
            points = points,
            selectedIndex = selectedIndex,
            mode = InsightsHeatmapMode.MONTH,
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
    points: List<InsightsChartPoint>,
    insightsMode: InsightsMode
): List<YearMonth> {
    val pointMonths = points.mapNotNull { point ->
        runCatching { YearMonth.from(LocalDate.parse(point.date)) }.getOrNull()
    }.distinct().sorted()
    if (pointMonths.isEmpty()) {
        return emptyList()
    }
    if (insightsMode != InsightsMode.YEAR) {
        return pointMonths
    }

    val year = pointMonths.last().year
    return (1..12).map { month -> YearMonth.of(year, month) }
}
