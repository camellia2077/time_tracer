package com.example.tracer

import androidx.annotation.StringRes
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.NavigationBar
import androidx.compose.material3.NavigationBarItem
import androidx.compose.material3.NavigationBarItemDefaults
import androidx.compose.material3.Scaffold
import androidx.compose.material3.SnackbarHost
import androidx.compose.material3.SnackbarHostState
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.testTag
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.dp

internal val ScreenOuterPadding: Dp = 16.dp

@StringRes
private fun treeResultTitle(period: DataTreePeriod): Int {
    return when (period) {
        DataTreePeriod.DAY -> R.string.tracer_tree_result_day
        DataTreePeriod.WEEK -> R.string.tracer_tree_result_week
        DataTreePeriod.MONTH -> R.string.tracer_tree_result_month
        DataTreePeriod.YEAR -> R.string.tracer_tree_result_year
        DataTreePeriod.RECENT -> R.string.tracer_tree_result_recent
        DataTreePeriod.RANGE -> R.string.tracer_tree_result_range
    }
}

@StringRes
private fun statsResultTitle(period: DataTreePeriod): Int {
    return when (period) {
        DataTreePeriod.DAY -> R.string.tracer_stats_result_day
        DataTreePeriod.WEEK -> R.string.tracer_stats_result_week
        DataTreePeriod.MONTH -> R.string.tracer_stats_result_month
        DataTreePeriod.YEAR -> R.string.tracer_stats_result_year
        DataTreePeriod.RECENT -> R.string.tracer_stats_result_recent
        DataTreePeriod.RANGE -> R.string.tracer_stats_result_range
    }
}

@Composable
internal fun TracerBottomNavShell(
    selectedTab: TracerTab,
    onTabSelected: (TracerTab) -> Unit,
    snackbarHostState: SnackbarHostState,
    content: @Composable (PaddingValues) -> Unit
) {
    Scaffold(
        snackbarHost = {
            SnackbarHost(hostState = snackbarHostState)
        },
        bottomBar = {
            val navItemColors = NavigationBarItemDefaults.colors(
                selectedIconColor = MaterialTheme.colorScheme.onPrimary,
                selectedTextColor = MaterialTheme.colorScheme.primary,
                unselectedIconColor = MaterialTheme.colorScheme.onSurfaceVariant,
                unselectedTextColor = MaterialTheme.colorScheme.onSurfaceVariant,
                indicatorColor = MaterialTheme.colorScheme.primary
            )

            NavigationBar(
                containerColor = MaterialTheme.colorScheme.surfaceContainer,
                tonalElevation = 0.dp
            ) {
                TracerTabRegistry.entries.forEach { entry ->
                    val tabMeta = entry.meta
                    val isSelected = selectedTab == tabMeta.id
                    val tabTitle = stringResource(tabMeta.titleRes)
                    NavigationBarItem(
                        modifier = if (tabMeta.testTag.isNullOrBlank()) {
                            Modifier
                        } else {
                            Modifier.testTag(tabMeta.testTag)
                        },
                        selected = isSelected,
                        onClick = { onTabSelected(tabMeta.id) },
                        icon = { Icon(tabMeta.icon, contentDescription = tabTitle) },
                        label = {
                            Text(
                                text = tabTitle,
                                style = MaterialTheme.typography.labelMedium.copy(
                                    fontWeight = if (isSelected) FontWeight.SemiBold else FontWeight.Medium
                                )
                            )
                        },
                        alwaysShowLabel = true,
                        colors = navItemColors
                    )
                }
            }
        },
        content = content
    )
}

@Composable
internal fun QueryResultDisplay(
    selectedTab: TracerTab,
    resultDisplayMode: ReportResultDisplayMode,
    activeResult: QueryResult?,
    analysisError: String,
    chartRoots: List<String>,
    chartSelectedRoot: String,
    chartDateInputMode: ChartDateInputMode,
    chartLookbackDays: String,
    chartRangeStartDate: String,
    chartRangeEndDate: String,
    chartLoading: Boolean,
    chartError: String,
    chartPoints: List<ReportChartPoint>,
    chartAverageDurationSeconds: Long?,
    chartUsesLegacyStatsFallback: Boolean,
    chartShowAverageLine: Boolean,
    onChartRootChange: (String) -> Unit,
    onChartDateInputModeChange: (ChartDateInputMode) -> Unit,
    onChartLookbackDaysChange: (String) -> Unit,
    onChartRangeStartDateChange: (String) -> Unit,
    onChartRangeEndDateChange: (String) -> Unit,
    onChartShowAverageLineChange: (Boolean) -> Unit,
    onLoadChart: () -> Unit
) {
    if (selectedTab != TracerTab.REPORT) {
        return
    }

    if (resultDisplayMode == ReportResultDisplayMode.CHART) {
        androidx.compose.material3.ElevatedCard(
            modifier = Modifier.fillMaxWidth()
        ) {
            Column(modifier = Modifier.padding(16.dp)) {
                Text(
                    text = stringResource(R.string.tracer_chart_result_title),
                    style = MaterialTheme.typography.titleMedium,
                    color = MaterialTheme.colorScheme.primary
                )
                Spacer(modifier = Modifier.height(8.dp))
                ReportChartResultContent(
                    chartRoots = chartRoots,
                    chartSelectedRoot = chartSelectedRoot,
                    chartDateInputMode = chartDateInputMode,
                    chartLookbackDays = chartLookbackDays,
                    chartRangeStartDate = chartRangeStartDate,
                    chartRangeEndDate = chartRangeEndDate,
                    chartLoading = chartLoading,
                    chartError = chartError,
                    chartPoints = chartPoints,
                    chartAverageDurationSeconds = chartAverageDurationSeconds,
                    chartUsesLegacyStatsFallback = chartUsesLegacyStatsFallback,
                    chartShowAverageLine = chartShowAverageLine,
                    onChartRootChange = onChartRootChange,
                    onChartDateInputModeChange = onChartDateInputModeChange,
                    onChartLookbackDaysChange = onChartLookbackDaysChange,
                    onChartRangeStartDateChange = onChartRangeStartDateChange,
                    onChartRangeEndDateChange = onChartRangeEndDateChange,
                    onChartShowAverageLineChange = onChartShowAverageLineChange,
                    onLoadChart = onLoadChart
                )
            }
        }
        return
    }

    if (activeResult != null) {
        androidx.compose.material3.ElevatedCard(
            modifier = Modifier.fillMaxWidth()
        ) {
            Column(modifier = Modifier.padding(16.dp)) {
                when (activeResult) {
                    is QueryResult.Report -> {
                        Text(
                            text = stringResource(R.string.tracer_report_result_title),
                            style = MaterialTheme.typography.titleMedium,
                            color = MaterialTheme.colorScheme.primary
                        )
                        Spacer(modifier = Modifier.height(8.dp))
                        com.example.tracer.ui.components.MarkdownText(
                            markdown = activeResult.text,
                            modifier = Modifier.fillMaxWidth()
                        )
                    }

                    is QueryResult.Stats -> {
                        Text(
                            text = stringResource(statsResultTitle(activeResult.period)),
                            style = MaterialTheme.typography.titleMedium,
                            color = MaterialTheme.colorScheme.primary
                        )
                        Spacer(modifier = Modifier.height(8.dp))
                        com.example.tracer.ui.components.MarkdownText(
                            markdown = activeResult.text,
                            modifier = Modifier.fillMaxWidth()
                        )
                    }

                    is QueryResult.Tree -> {
                        Text(
                            text = stringResource(treeResultTitle(activeResult.period)),
                            style = MaterialTheme.typography.titleMedium,
                            color = MaterialTheme.colorScheme.primary
                        )
                        Spacer(modifier = Modifier.height(8.dp))
                        Text(
                            text = activeResult.text,
                            style = MaterialTheme.typography.bodyMedium,
                            fontFamily = FontFamily.Monospace,
                            modifier = Modifier.fillMaxWidth()
                        )
                    }
                }
            }
        }
    }
    if (analysisError.isNotBlank()) {
        androidx.compose.material3.ElevatedCard(
            modifier = Modifier.fillMaxWidth()
        ) {
            Column(modifier = Modifier.padding(16.dp)) {
                Text(
                    text = stringResource(R.string.tracer_analysis_error_title),
                    style = MaterialTheme.typography.titleMedium,
                    color = MaterialTheme.colorScheme.error
                )
                Spacer(modifier = Modifier.height(8.dp))
                Text(
                    text = analysisError,
                    style = MaterialTheme.typography.bodyMedium,
                    modifier = Modifier.fillMaxWidth()
                )
            }
        }
    }
}
