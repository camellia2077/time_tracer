package com.example.tracer

import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.heightIn
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ArrowDownward
import androidx.compose.material.icons.filled.ArrowUpward
import androidx.compose.material.icons.filled.Remove
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Switch
import androidx.compose.material3.SegmentedButton
import androidx.compose.material3.SegmentedButtonDefaults
import androidx.compose.material3.SingleChoiceSegmentedButtonRow
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.insights.R
import com.example.tracer.ui.components.CalendarAvailability
import com.example.tracer.ui.components.TracerSegmentedButtonDefaults

/**
 * Text-mode activity browser for a multi-day insights window. It keeps the
 * time-independent period overview separate from progressively disclosed
 * records, so long month, year, and range windows do not render every row at
 * once.
 */
@Composable
internal fun InsightsPeriodActivityBrowser(
    activityDays: List<StructuredDailyInsights>,
    projectTree: List<StructuredInsightsProjectNode>,
    insightsMode: InsightsMode,
    periodComparison: InsightsPeriodComparisonState = InsightsPeriodComparisonState.Hidden,
    canComparePreviousPeriod: Boolean = false,
    calendarAvailability: CalendarAvailability,
    selectedView: InsightsActivityView,
    onSelectedViewChange: (InsightsActivityView) -> Unit,
    onPeriodComparisonToggle: () -> Unit = {},
    onComparisonPeriodSelected: (InsightsPeriodSelection) -> Unit = {},
    modifier: Modifier = Modifier
) {
    Column(
        modifier = modifier,
        verticalArrangement = Arrangement.spacedBy(8.dp)
    ) {
        Text(
            text = stringResource(R.string.insights_result_title_period_activities),
            style = MaterialTheme.typography.titleMedium,
            color = MaterialTheme.colorScheme.primary
        )
        InsightsActivityViewSwitcher(
            selectedView = selectedView,
            views = listOf(InsightsActivityView.OVERVIEW, InsightsActivityView.RECORDS),
            onSelect = onSelectedViewChange
        )
        if (activityDays.isEmpty() && selectedView != InsightsActivityView.OVERVIEW) {
            Text(
                text = stringResource(R.string.insights_period_activities_empty),
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
        } else if (selectedView == InsightsActivityView.OVERVIEW) {
            InsightsActivityOverview(
                activityDays = activityDays,
                projectTree = projectTree,
                insightsMode = insightsMode,
                periodComparison = periodComparison,
                canComparePreviousPeriod = canComparePreviousPeriod,
                calendarAvailability = calendarAvailability,
                onPeriodComparisonToggle = onPeriodComparisonToggle,
                onComparisonPeriodSelected = onComparisonPeriodSelected
            )
        } else {
            InsightsPeriodActivityRecords(activityDays = activityDays)
        }
    }
}

enum class InsightsActivityView {
    OVERVIEW,
    RECORDS
}

@Composable
internal fun InsightsActivityViewSwitcher(
    selectedView: InsightsActivityView,
    views: List<InsightsActivityView>,
    onSelect: (InsightsActivityView) -> Unit
) {
    SingleChoiceSegmentedButtonRow(modifier = Modifier.fillMaxWidth()) {
        views.forEachIndexed { index, view ->
            val selected = selectedView == view
            SegmentedButton(
                shape = SegmentedButtonDefaults.itemShape(index, views.size),
                onClick = { onSelect(view) },
                selected = selected,
                colors = TracerSegmentedButtonDefaults.colors(),
                label = {
                    Text(
                        text = stringResource(
                            if (view == InsightsActivityView.OVERVIEW) {
                                R.string.insights_period_activities_overview
                            } else {
                                R.string.insights_period_activities_records
                            }
                        ),
                        fontWeight = if (selected) {
                            TracerSegmentedButtonDefaults.activeLabelFontWeight
                        } else {
                            TracerSegmentedButtonDefaults.inactiveLabelFontWeight
                        }
                    )
                }
            )
        }
    }
}

@Composable
internal fun InsightsActivityOverview(
    activityDays: List<StructuredDailyInsights>,
    projectTree: List<StructuredInsightsProjectNode>,
    insightsMode: InsightsMode,
    periodComparison: InsightsPeriodComparisonState = InsightsPeriodComparisonState.Hidden,
    canComparePreviousPeriod: Boolean = false,
    calendarAvailability: CalendarAvailability,
    onPeriodComparisonToggle: () -> Unit = {},
    onComparisonPeriodSelected: (InsightsPeriodSelection) -> Unit = {}
) {
    val comparisonColors = insightsSemanticColors()
    val totalDuration = activityDays.sumOf { it.totalDurationSeconds }
    val recordCount = activityDays.sumOf { it.activities.size }
    val activeDayCount = activityDays.size
    val averageDuration = if (activeDayCount == 0) 0L else totalDuration / activeDayCount
    val previous = periodComparison as? InsightsPeriodComparisonState.Ready
    val previousTotalDuration = previous?.activityDays?.sumOf { it.totalDurationSeconds }
    val previousRecordCount = previous?.activityDays?.sumOf { it.activities.size }
    val previousActiveDayCount = previous?.activityDays?.size
    val previousAverageDuration = previous?.let {
        val activeDays = it.activityDays.size
        if (activeDays == 0) 0L else it.activityDays.sumOf(StructuredDailyInsights::totalDurationSeconds) / activeDays
    }
    Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
        InsightsPeriodComparisonControl(
            periodComparison = periodComparison,
            canComparePreviousPeriod = canComparePreviousPeriod,
            insightsMode = insightsMode,
            calendarAvailability = calendarAvailability,
            onPeriodComparisonToggle = onPeriodComparisonToggle,
            onComparisonPeriodSelected = onComparisonPeriodSelected
        )
        previous?.let {
            Text(
                text = stringResource(
                    R.string.insights_period_activities_comparison_period,
                    it.label
                ),
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
        }
        Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            PeriodActivityMetric(
                label = stringResource(R.string.insights_period_activities_total),
                value = formatPeriodActivityDuration(totalDuration),
                comparison = previousTotalDuration?.let {
                    periodActivityDurationComparison(totalDuration, it)
                },
                comparisonColors = comparisonColors,
                modifier = Modifier.weight(1f)
            )
            PeriodActivityMetric(
                label = stringResource(R.string.insights_period_activities_active_days),
                value = activeDayCount.toString(),
                comparison = previousActiveDayCount?.let {
                    periodActivityCountComparison(activeDayCount.toLong(), it.toLong())
                },
                comparisonColors = comparisonColors,
                modifier = Modifier.weight(1f)
            )
        }
        Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            PeriodActivityMetric(
                label = stringResource(R.string.insights_period_activities_record_count),
                value = recordCount.toString(),
                comparison = previousRecordCount?.let {
                    periodActivityCountComparison(recordCount.toLong(), it.toLong())
                },
                comparisonColors = comparisonColors,
                modifier = Modifier.weight(1f)
            )
            PeriodActivityMetric(
                label = stringResource(R.string.insights_period_activities_daily_average),
                value = formatPeriodActivityDuration(averageDuration),
                comparison = previousAverageDuration?.let {
                    periodActivityDurationComparison(averageDuration, it)
                },
                comparisonColors = comparisonColors,
                modifier = Modifier.weight(1f)
            )
        }
        if (projectTree.isNotEmpty()) {
            Text(
                text = stringResource(R.string.insights_period_activities_parent_breakdown),
                style = MaterialTheme.typography.titleSmall,
                color = MaterialTheme.colorScheme.onSurface
            )
            Surface(
                modifier = Modifier.fillMaxWidth(),
                shape = MaterialTheme.shapes.medium,
                color = MaterialTheme.colorScheme.surfaceContainerLow
            ) {
                Column(modifier = Modifier.padding(12.dp)) {
                    projectTree
                        .sortedByDescending(StructuredInsightsProjectNode::durationSeconds)
                        .forEach { node ->
                            InsightsPeriodActivityProjectNode(
                                node = node,
                                parentDuration = totalDuration,
                                previousNode = previous?.projectTree?.firstOrNull { it.name == node.name },
                                showComparison = previous != null,
                                comparisonColors = comparisonColors
                            )
                        }
                }
            }
        }
    }
}

@Composable
private fun InsightsPeriodComparisonControl(
    periodComparison: InsightsPeriodComparisonState,
    canComparePreviousPeriod: Boolean,
    insightsMode: InsightsMode,
    calendarAvailability: CalendarAvailability,
    onPeriodComparisonToggle: () -> Unit,
    onComparisonPeriodSelected: (InsightsPeriodSelection) -> Unit
) {
    val enabled = canComparePreviousPeriod && periodComparison !is InsightsPeriodComparisonState.Loading
    val comparing = periodComparison !is InsightsPeriodComparisonState.Hidden
    Row(
        modifier = Modifier.fillMaxWidth(),
        verticalAlignment = Alignment.CenterVertically
    ) {
        Text(
            text = stringResource(
                R.string.insights_period_activities_compare_previous
            ),
            modifier = Modifier.weight(1f),
            style = MaterialTheme.typography.titleSmall
        )
        Switch(
            checked = comparing,
            onCheckedChange = { onPeriodComparisonToggle() },
            enabled = enabled
        )
    }
    if (periodComparison !is InsightsPeriodComparisonState.Hidden) {
        InsightsComparisonPeriodPicker(
            insightsMode = insightsMode,
            periodComparison = periodComparison,
            calendarAvailability = calendarAvailability,
            onPeriodSelected = onComparisonPeriodSelected
        )
    }
    when (periodComparison) {
        is InsightsPeriodComparisonState.Loading -> Text(
            text = stringResource(R.string.insights_period_activities_comparison_loading),
            style = MaterialTheme.typography.bodyMedium,
            color = MaterialTheme.colorScheme.onSurfaceVariant
        )
        is InsightsPeriodComparisonState.Ready -> Unit
        is InsightsPeriodComparisonState.Failed -> Text(
            text = periodComparison.message,
            style = MaterialTheme.typography.bodyMedium,
            color = MaterialTheme.colorScheme.error
        )
        InsightsPeriodComparisonState.Hidden -> Unit
    }
}

@Composable
private fun InsightsPeriodActivityProjectNode(
    node: StructuredInsightsProjectNode,
    parentDuration: Long,
    previousNode: StructuredInsightsProjectNode? = null,
    showComparison: Boolean = false,
    comparisonColors: InsightsSemanticColors,
    depth: Int = 0
) {
    val hasChildren = node.children.isNotEmpty()
    var expanded by remember(node.name, depth) { mutableStateOf(false) }
    Column(
        modifier = Modifier
            .fillMaxWidth()
            .clickable(enabled = hasChildren) { expanded = !expanded }
            .padding(start = (depth * 16).dp, end = 4.dp)
    ) {
        Row(
            modifier = Modifier.heightIn(min = 48.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            if (hasChildren) {
                Text(
                    text = if (expanded) "▼" else "▶",
                    modifier = Modifier.width(24.dp),
                    color = MaterialTheme.colorScheme.primary,
                    style = MaterialTheme.typography.titleMedium
                )
            } else {
                Spacer(modifier = Modifier.width(24.dp))
            }
            Text(
                text = node.name,
                modifier = Modifier.weight(1f),
                style = if (depth == 0) {
                    MaterialTheme.typography.titleMedium
                } else {
                    MaterialTheme.typography.bodyLarge
                }
            )
            Text(
                text = formatParentActivityDuration(node.durationSeconds),
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.primary
            )
            Spacer(modifier = Modifier.width(8.dp))
            Text(
                text = formatPeriodActivityShare(node.durationSeconds, parentDuration),
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
        }
        if (showComparison) {
            PeriodActivityComparisonLine(
                comparison = periodActivityDurationComparison(
                    current = node.durationSeconds,
                    previous = previousNode?.durationSeconds ?: 0L
                ),
                colors = comparisonColors,
                modifier = Modifier.padding(start = 24.dp, bottom = 8.dp)
            )
        }
    }
    if (expanded) {
        node.children
            .sortedByDescending(StructuredInsightsProjectNode::durationSeconds)
            .forEach { child ->
                InsightsPeriodActivityProjectNode(
                    node = child,
                    parentDuration = node.durationSeconds,
                    previousNode = previousNode?.children?.firstOrNull { it.name == child.name },
                    showComparison = showComparison,
                    comparisonColors = comparisonColors,
                    depth = depth + 1
                )
            }
    }
}

@Composable
private fun PeriodActivityMetric(
    label: String,
    value: String,
    comparison: PeriodActivityComparison? = null,
    comparisonColors: InsightsSemanticColors,
    modifier: Modifier = Modifier
) {
    Surface(
        modifier = modifier,
        shape = MaterialTheme.shapes.medium,
        color = MaterialTheme.colorScheme.surfaceContainerLow
    ) {
        Column(modifier = Modifier.padding(12.dp)) {
            Text(
                text = label,
                style = MaterialTheme.typography.labelMedium,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
            Text(
                text = value,
                style = MaterialTheme.typography.titleMedium,
                color = MaterialTheme.colorScheme.primary
            )
            comparison?.let {
                PeriodActivityComparisonLine(comparison = it, colors = comparisonColors)
            }
        }
    }
}

@Composable
private fun InsightsPeriodActivityRecords(activityDays: List<StructuredDailyInsights>) {
    val daysByMonth = remember(activityDays) {
        sortPeriodActivityDaysChronologically(activityDays)
            .groupBy { day -> day.date.substringBeforeLast('-', day.date) }
    }
    if (daysByMonth.size == 1) {
        daysByMonth.values.single().forEach { day ->
            InsightsPeriodActivityDay(day = day)
        }
    } else {
        daysByMonth.forEach { (month, days) ->
            InsightsPeriodActivityMonth(month = month, days = days)
        }
    }
}

@Composable
private fun InsightsPeriodActivityMonth(month: String, days: List<StructuredDailyInsights>) {
    var expanded by remember(month) { mutableStateOf(false) }
    val totalDuration = days.sumOf { it.totalDurationSeconds }
    val recordCount = days.sumOf { it.activities.size }
    InsightsTimelineExpandableGroup(
        expanded = expanded,
        onToggle = { expanded = !expanded },
        title = month,
        trailing = formatPeriodActivityDuration(totalDuration),
        summary = stringResource(
            R.string.insights_period_activities_month_summary,
            days.size,
            recordCount
        )
    ) {
        days.forEach { day -> InsightsPeriodActivityDay(day = day) }
    }
}

@Composable
private fun InsightsPeriodActivityDay(day: StructuredDailyInsights) {
    var expanded by remember(day.date) { mutableStateOf(false) }
    InsightsTimelineExpandableGroup(
        expanded = expanded,
        onToggle = { expanded = !expanded },
        title = day.date,
        trailing = formatPeriodActivityDuration(day.totalDurationSeconds),
        summary = stringResource(
            R.string.insights_period_activities_day_summary,
            day.activities.size
        )
    ) {
        InsightsTimelineRecordList(
            activities = day.activities,
            layout = InsightsTimelineLayout.FIXED
        )
    }
}

internal fun sortPeriodActivityDaysChronologically(
    activityDays: List<StructuredDailyInsights>
): List<StructuredDailyInsights> = activityDays.sortedBy(StructuredDailyInsights::date)

internal fun formatPeriodActivityDuration(durationSeconds: Long): String {
    return formatInsightsDuration(durationSeconds, InsightsDurationFormat.COMPACT)
}

private fun formatParentActivityDuration(durationSeconds: Long): String {
    return formatInsightsDuration(durationSeconds, InsightsDurationFormat.COMPACT)
}

private fun formatPeriodActivityShare(durationSeconds: Long, totalDuration: Long): String {
    if (totalDuration <= 0L) return "0%"
    val percent = (durationSeconds.coerceAtLeast(0L) * 100L) / totalDuration
    return "$percent%"
}

private sealed interface PeriodActivityComparison {
    data class Increase(val delta: String, val percentage: String) : PeriodActivityComparison
    data class Decrease(val delta: String, val percentage: String) : PeriodActivityComparison
    data class New(val delta: String) : PeriodActivityComparison
    data class Neutral(val delta: String) : PeriodActivityComparison
}

private fun periodActivityDurationComparison(current: Long, previous: Long): PeriodActivityComparison =
    periodActivityComparison(
        current = current,
        previous = previous,
        value = { duration -> formatPeriodActivityDuration(duration) }
    )

private fun periodActivityCountComparison(current: Long, previous: Long): PeriodActivityComparison =
    periodActivityComparison(current = current, previous = previous, value = Long::toString)

private fun periodActivityComparison(
    current: Long,
    previous: Long,
    value: (Long) -> String
): PeriodActivityComparison {
    val delta = current - previous
    if (previous == 0L && current > 0L) {
        return PeriodActivityComparison.New(delta = "+${value(current)}")
    }
    if (delta == 0L) return PeriodActivityComparison.Neutral(delta = value(0L))

    val percentage = kotlin.math.round(
        (delta.toDouble() * 100.0) / previous.toDouble()
    ).toLong()
    return if (delta > 0L) {
        PeriodActivityComparison.Increase(
            delta = "+${value(delta)}",
            percentage = "+$percentage%"
        )
    } else {
        PeriodActivityComparison.Decrease(
            delta = "-${value(-delta)}",
            percentage = "$percentage%"
        )
    }
}

@Composable
private fun PeriodActivityComparisonLine(
    comparison: PeriodActivityComparison,
    colors: InsightsSemanticColors,
    modifier: Modifier = Modifier
) {
    val (icon, text, color) = when (comparison) {
        is PeriodActivityComparison.Increase -> Triple(
            Icons.Filled.ArrowUpward,
            stringResource(
                R.string.insights_period_activities_comparison_increase,
                comparison.delta,
                comparison.percentage
            ),
            colors.comparisonIncrease
        )
        is PeriodActivityComparison.Decrease -> Triple(
            Icons.Filled.ArrowDownward,
            stringResource(
                R.string.insights_period_activities_comparison_decrease,
                comparison.delta,
                comparison.percentage
            ),
            colors.comparisonDecrease
        )
        is PeriodActivityComparison.New -> Triple(
            Icons.Filled.ArrowUpward,
            stringResource(
                R.string.insights_period_activities_comparison_new_value,
                comparison.delta
            ),
            colors.comparisonIncrease
        )
        is PeriodActivityComparison.Neutral -> Triple(
            Icons.Filled.Remove,
            stringResource(
                R.string.insights_period_activities_comparison_neutral, comparison.delta),
            colors.comparisonNeutral
        )
    }
    Row(
        modifier = modifier,
        verticalAlignment = Alignment.CenterVertically
    ) {
        Icon(
            imageVector = icon,
            contentDescription = null,
            modifier = Modifier.width(16.dp),
            tint = color
        )
        Spacer(modifier = Modifier.width(4.dp))
        Text(
            text = text,
            style = MaterialTheme.typography.bodySmall,
            color = color
        )
    }
}
