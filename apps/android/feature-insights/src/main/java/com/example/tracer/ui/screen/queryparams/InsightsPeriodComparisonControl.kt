package com.example.tracer

import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.Column
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Switch
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.Alignment
import androidx.compose.ui.res.stringResource
import com.example.tracer.feature.insights.R
import com.example.tracer.ui.components.CalendarAvailability

/** Shared comparison toggle and period picker used by Activities and Trend charts. */
@Composable
internal fun InsightsPeriodComparisonControl(
    periodComparison: InsightsPeriodComparisonState,
    canComparePreviousPeriod: Boolean,
    insightsMode: InsightsMode,
    calendarAvailability: CalendarAvailability,
    onPeriodComparisonToggle: () -> Unit,
    onComparisonPeriodSelected: (InsightsPeriodSelection) -> Unit,
    modifier: Modifier = Modifier
) {
    val enabled = canComparePreviousPeriod &&
        periodComparison !is InsightsPeriodComparisonState.Loading
    val comparing = periodComparison !is InsightsPeriodComparisonState.Hidden
    Column(modifier = modifier.fillMaxWidth()) {
        Row(
            modifier = Modifier.fillMaxWidth(),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Text(
                text = stringResource(R.string.insights_period_activities_compare_previous),
                modifier = Modifier.weight(1f),
                style = MaterialTheme.typography.titleSmall
            )
            Switch(
                checked = comparing,
                onCheckedChange = { onPeriodComparisonToggle() },
                enabled = enabled
            )
        }
        // Keep the period selector hidden while the comparison query is loading. The selected
        // period is only actionable once the comparison result has been loaded successfully.
        if (periodComparison is InsightsPeriodComparisonState.Ready) {
            InsightsComparisonPeriodPicker(
                insightsMode = insightsMode,
                periodComparison = periodComparison,
                calendarAvailability = calendarAvailability,
                onPeriodSelected = onComparisonPeriodSelected
            )
        }
        when (periodComparison) {
            is InsightsPeriodComparisonState.Loading -> Unit
            is InsightsPeriodComparisonState.Ready -> Unit
            is InsightsPeriodComparisonState.Failed -> Text(
                text = periodComparison.message,
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.error
            )
            InsightsPeriodComparisonState.Hidden -> Unit
        }
    }
}
