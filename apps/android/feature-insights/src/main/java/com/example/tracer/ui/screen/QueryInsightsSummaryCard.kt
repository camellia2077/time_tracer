package com.example.tracer

import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.ElevatedCard
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.insights.R

@Composable
internal fun QueryInsightsSummaryCard(
    summary: InsightsSummary,
    modifier: Modifier = Modifier
) {
    ElevatedCard(modifier = modifier) {
        Column(modifier = Modifier.padding(16.dp)) {
            when (summary) {
                is InsightsSummary.NoData -> {
                    val periodLabel = stringResource(summary.period.insightsModeResId())
                    Text(
                        text = stringResource(
                            R.string.insights_result_title_insights_no_data,
                            periodLabel
                        ),
                        style = MaterialTheme.typography.titleMedium,
                        color = MaterialTheme.colorScheme.primary
                    )
                    Spacer(modifier = Modifier.height(8.dp))
                    Text(
                        text = stringResource(
                            R.string.insights_summary_no_data_body,
                            periodLabel
                        ),
                        style = MaterialTheme.typography.bodyMedium
                    )
                }

                is InsightsSummary.MissingTarget -> {
                    val periodLabel = stringResource(summary.period.insightsModeResId())
                    Text(
                        text = stringResource(
                            R.string.insights_result_title_insights_missing_target,
                            periodLabel
                        ),
                        style = MaterialTheme.typography.titleMedium,
                        color = MaterialTheme.colorScheme.error
                    )
                    Spacer(modifier = Modifier.height(8.dp))
                    Text(
                        text = stringResource(
                            R.string.insights_summary_missing_target_body,
                            periodLabel
                        ),
                        style = MaterialTheme.typography.bodyMedium
                    )
                    if (summary.errorCode.isNotBlank()) {
                        QueryInsightsSummaryLine(
                            text = stringResource(
                                R.string.insights_summary_error_code,
                                summary.errorCode
                            )
                        )
                    }
                    if (summary.errorCategory.isNotBlank()) {
                        QueryInsightsSummaryLine(
                            text = stringResource(
                                R.string.insights_summary_error_category,
                                summary.errorCategory
                            )
                        )
                    }
                    if (summary.hints.isNotEmpty()) {
                        QueryInsightsSummaryLine(
                            text = stringResource(
                                R.string.insights_summary_hints,
                                summary.hints.joinToString(separator = " | ")
                            )
                        )
                    }
                }

                is InsightsSummary.WindowMetadata -> {
                    val periodLabel = stringResource(summary.period.insightsModeResId())
                    val metadata = summary.metadata
                    Text(
                        text = stringResource(
                            R.string.insights_result_title_insights_window_summary,
                            periodLabel
                        ),
                        style = MaterialTheme.typography.titleMedium,
                        color = MaterialTheme.colorScheme.primary
                    )
                    Spacer(modifier = Modifier.height(8.dp))
                    Text(
                        text = if (metadata.hasRecords) {
                            stringResource(
                                R.string.insights_summary_window_has_records_body,
                                periodLabel
                            )
                        } else {
                            stringResource(
                                R.string.insights_summary_window_empty_body,
                                periodLabel
                            )
                        },
                        style = MaterialTheme.typography.bodyMedium
                    )
                    if (metadata.startDate.isNotBlank() || metadata.endDate.isNotBlank()) {
                        QueryInsightsSummaryLine(
                            text = stringResource(
                                R.string.insights_summary_window_range,
                                metadata.startDate.ifBlank { "-" },
                                metadata.endDate.ifBlank { "-" }
                            )
                        )
                    }
                    if (metadata.requestedDays > 0) {
                        QueryInsightsSummaryLine(
                            text = stringResource(
                                R.string.insights_summary_requested_days,
                                metadata.requestedDays
                            )
                        )
                    }
                    QueryInsightsSummaryLine(
                        text = stringResource(
                            R.string.insights_summary_matched_days,
                            metadata.matchedDayCount
                        )
                    )
                    QueryInsightsSummaryLine(
                        text = stringResource(
                            R.string.insights_summary_matched_records,
                            metadata.matchedRecordCount
                        )
                    )
                }
            }
        }
    }
}

@Composable
private fun QueryInsightsSummaryLine(text: String) {
    Spacer(modifier = Modifier.height(6.dp))
    Text(
        text = text,
        style = MaterialTheme.typography.bodySmall,
        modifier = Modifier.fillMaxWidth()
    )
}

internal fun DataTreePeriod.insightsModeResId(): Int {
    return when (this) {
        DataTreePeriod.DAY -> R.string.insights_mode_day
        DataTreePeriod.WEEK -> R.string.insights_mode_week
        DataTreePeriod.MONTH -> R.string.insights_mode_month
        DataTreePeriod.YEAR -> R.string.insights_mode_year
        DataTreePeriod.RECENT -> R.string.insights_mode_recent
        DataTreePeriod.RANGE -> R.string.insights_mode_range
    }
}
