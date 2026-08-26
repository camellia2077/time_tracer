package com.example.tracer

import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.outlined.Info
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.ModalBottomSheet
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.material3.rememberModalBottomSheetState
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
import androidx.annotation.StringRes
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
    chartMedianDurationSeconds: Double?,
    chartMinimumDurationSeconds: Double?,
    chartMaximumDurationSeconds: Double?,
    chartLowerQuartileDurationSeconds: Double?,
    chartUpperQuartileDurationSeconds: Double?,
    chartCoefficientOfVariation: Double?,
    chartMeanAbsoluteDeviationSeconds: Double?
) {
    var visibleExplanation by remember { mutableStateOf<VariationExplanation?>(null) }
    val selectedPoint = sortedChartPoints.getOrNull(selectedPointIndex)
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
    val averageDurationSeconds = chartAverageDurationSeconds.coerceAtLeast(0L)
    val usualLowerBound = chartLowerQuartileDurationSeconds?.roundToLong()
    val usualUpperBound = chartUpperQuartileDurationSeconds?.roundToLong()

    Column(
        modifier = Modifier
            .fillMaxWidth()
            .padding(top = 8.dp),
        verticalArrangement = Arrangement.spacedBy(16.dp)
    ) {
        if (selectedPoint != null && usualLowerBound != null && usualUpperBound != null) {
            val insight = when {
                selectedPoint.durationSeconds > usualUpperBound -> R.string.insights_chart_selected_above_usual
                selectedPoint.durationSeconds < usualLowerBound -> R.string.insights_chart_selected_below_usual
                else -> R.string.insights_chart_selected_within_usual
            }
            Text(
                text = stringResource(
                    insight,
                    formatDurationHoursMinutes(selectedPoint.durationSeconds),
                    formatDurationHoursMinutes(usualLowerBound),
                    formatDurationHoursMinutes(usualUpperBound)
                ),
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.onSurfaceVariant,
                fontWeight = FontWeight.Normal
            )
        }

        TrendSummaryGroup(title = stringResource(R.string.insights_chart_section_overview)) {
            selectedPoint?.let { point ->
                Text(
                    text = stringResource(
                        R.string.insights_chart_selected_detail,
                        point.date,
                        formatDurationHoursMinutes(point.durationSeconds)
                    ),
                    style = MaterialTheme.typography.bodyMedium,
                    color = MaterialTheme.colorScheme.onSurfaceVariant,
                    fontWeight = FontWeight.Normal
                )
            }
            if (totalDays > 0) {
                TrendSummaryMetric(
                    label = stringResource(R.string.insights_chart_label_coverage),
                    value = stringResource(
                        R.string.insights_chart_record_completeness,
                        recordedDays,
                        totalDays,
                        (recordedDays * 100.0 / totalDays).toInt()
                    )
                )
            }
            TrendSummaryMetric(
                label = stringResource(R.string.insights_chart_label_total_time),
                value = formatDurationHoursMinutes(chartTotalDurationSeconds.coerceAtLeast(0L)),
                emphasizeValue = true
            )
            TrendSummaryMetric(
                label = stringResource(R.string.insights_chart_label_records),
                value = chartTotalOccurrenceCount.coerceAtLeast(0L).toString()
            )
        }

        HorizontalDivider(color = MaterialTheme.colorScheme.outline.copy(alpha = 0.35f))

        TrendSummaryGroup(title = stringResource(R.string.insights_chart_section_typical_day)) {
            TrendSummaryMetric(
                label = stringResource(R.string.insights_chart_label_daily_average),
                value = formatDurationHoursMinutes(averageDurationSeconds),
                emphasizeValue = true
            )
            chartMedianDurationSeconds?.let { durationSeconds ->
                TrendSummaryMetric(
                    label = stringResource(R.string.insights_chart_label_median_day),
                    value = formatDurationHoursMinutes(durationSeconds.roundToLong())
                )
            }
            if (usualLowerBound != null && usualUpperBound != null) {
                TrendSummaryMetric(
                    label = stringResource(R.string.insights_chart_label_usual_daily_band),
                    value = "${formatDurationHoursMinutes(usualLowerBound)}–" +
                        formatDurationHoursMinutes(usualUpperBound)
                )
            }
            TrendSummaryMetric(
                label = stringResource(R.string.insights_chart_label_average_per_record),
                value = formatDurationHoursMinutes(
                    chartAverageDurationPerOccurrenceSeconds.coerceAtLeast(0L)
                )
            )
        }

        HorizontalDivider(color = MaterialTheme.colorScheme.outline.copy(alpha = 0.35f))

        TrendSummaryGroup(title = stringResource(R.string.insights_chart_section_variation)) {
            if (chartMinimumDurationSeconds != null && chartMaximumDurationSeconds != null) {
                TrendSummaryMetric(
                    label = stringResource(R.string.insights_chart_label_lowest_highest_day),
                    value = "${formatDurationHoursMinutes(chartMinimumDurationSeconds.roundToLong())}–" +
                        formatDurationHoursMinutes(chartMaximumDurationSeconds.roundToLong())
                )
            }
            chartCoefficientOfVariation?.let { coefficient ->
                val value = String.format(Locale.ROOT, "%.0f%%", coefficient * 100.0)
                TrendSummaryMetric(
                    label = stringResource(R.string.insights_chart_label_relative_variation),
                    value = value,
                    explanation = TrendSummaryMetricExplanation(
                        contentDescription = stringResource(
                            R.string.insights_chart_show_explanation,
                            stringResource(R.string.insights_chart_label_relative_variation)
                        ),
                        onClick = {
                            visibleExplanation = VariationExplanation(
                                titleRes = R.string.insights_chart_label_relative_variation,
                                bodyRes = R.string.insights_chart_relative_variation_explanation,
                                value = value
                            )
                        }
                    )
                )
            }
            chartMeanAbsoluteDeviationSeconds?.let { durationSeconds ->
                val value = formatDurationHoursMinutes(durationSeconds.roundToLong())
                TrendSummaryMetric(
                    label = stringResource(R.string.insights_chart_label_typical_distance),
                    value = value,
                    explanation = TrendSummaryMetricExplanation(
                        contentDescription = stringResource(
                            R.string.insights_chart_show_explanation,
                            stringResource(R.string.insights_chart_label_typical_distance)
                        ),
                        onClick = {
                            visibleExplanation = VariationExplanation(
                                titleRes = R.string.insights_chart_label_typical_distance,
                                bodyRes = R.string.insights_chart_typical_distance_explanation,
                                value = value
                            )
                        }
                    )
                )
            }
        }
    }

    visibleExplanation?.let { explanation ->
        VariationExplanationSheet(
            explanation = explanation,
            onDismiss = { visibleExplanation = null }
        )
    }
}

private data class VariationExplanation(
    @param:StringRes val titleRes: Int,
    @param:StringRes val bodyRes: Int,
    val value: String
)

@OptIn(ExperimentalMaterial3Api::class)
@Composable
private fun VariationExplanationSheet(
    explanation: VariationExplanation,
    onDismiss: () -> Unit
) {
    val title = stringResource(explanation.titleRes)
    val body = stringResource(explanation.bodyRes, explanation.value)
    ModalBottomSheet(
        onDismissRequest = onDismiss,
        sheetState = rememberModalBottomSheetState(skipPartiallyExpanded = true)
    ) {
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 24.dp, vertical = 8.dp),
            verticalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            Text(text = title, style = MaterialTheme.typography.titleMedium)
            Text(
                text = body,
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
            TextButton(
                modifier = Modifier.align(Alignment.End),
                onClick = onDismiss
            ) {
                Text(stringResource(R.string.insights_chart_explanation_dismiss))
            }
        }
    }
}

@Composable
private fun TrendSummaryGroup(
    title: String,
    content: @Composable () -> Unit
) {
    Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
        Text(
            text = title,
            style = MaterialTheme.typography.titleSmall,
            color = MaterialTheme.colorScheme.primary
        )
        content()
    }
}

@Composable
private fun TrendSummaryMetric(
    label: String,
    value: String,
    emphasizeValue: Boolean = false,
    explanation: TrendSummaryMetricExplanation? = null
) {
    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.SpaceBetween,
        verticalAlignment = Alignment.CenterVertically
    ) {
        Text(
            text = label,
            modifier = Modifier.weight(1f),
            style = MaterialTheme.typography.bodySmall,
            color = MaterialTheme.colorScheme.onSurfaceVariant
        )
        Row(verticalAlignment = Alignment.CenterVertically) {
            Text(
                text = value,
                style = if (emphasizeValue) {
                    MaterialTheme.typography.bodyMedium
                } else {
                    MaterialTheme.typography.bodySmall
                },
                fontWeight = if (emphasizeValue) FontWeight.SemiBold else FontWeight.Normal,
                color = MaterialTheme.colorScheme.onSurface
            )
            explanation?.let { action ->
                IconButton(
                    onClick = action.onClick,
                    modifier = Modifier.size(40.dp)
                ) {
                    Icon(
                        imageVector = Icons.Outlined.Info,
                        contentDescription = action.contentDescription,
                        tint = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                }
            }
        }
    }
}

private data class TrendSummaryMetricExplanation(
    val contentDescription: String,
    val onClick: () -> Unit
)
