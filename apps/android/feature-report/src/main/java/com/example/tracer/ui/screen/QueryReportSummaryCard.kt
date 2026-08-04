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
import com.example.tracer.feature.report.R

@Composable
internal fun QueryReportSummaryCard(
    summary: ReportSummary,
    modifier: Modifier = Modifier
) {
    ElevatedCard(modifier = modifier) {
        Column(modifier = Modifier.padding(16.dp)) {
            when (summary) {
                is ReportSummary.NoData -> {
                    val periodLabel = stringResource(summary.period.reportModeResId())
                    Text(
                        text = stringResource(
                            R.string.report_result_title_report_no_data,
                            periodLabel
                        ),
                        style = MaterialTheme.typography.titleMedium,
                        color = MaterialTheme.colorScheme.primary
                    )
                    Spacer(modifier = Modifier.height(8.dp))
                    Text(
                        text = stringResource(
                            R.string.report_summary_no_data_body,
                            periodLabel
                        ),
                        style = MaterialTheme.typography.bodyMedium
                    )
                }

                is ReportSummary.MissingTarget -> {
                    val periodLabel = stringResource(summary.period.reportModeResId())
                    Text(
                        text = stringResource(
                            R.string.report_result_title_report_missing_target,
                            periodLabel
                        ),
                        style = MaterialTheme.typography.titleMedium,
                        color = MaterialTheme.colorScheme.error
                    )
                    Spacer(modifier = Modifier.height(8.dp))
                    Text(
                        text = stringResource(
                            R.string.report_summary_missing_target_body,
                            periodLabel
                        ),
                        style = MaterialTheme.typography.bodyMedium
                    )
                    if (summary.errorCode.isNotBlank()) {
                        QueryReportSummaryLine(
                            text = stringResource(
                                R.string.report_summary_error_code,
                                summary.errorCode
                            )
                        )
                    }
                    if (summary.errorCategory.isNotBlank()) {
                        QueryReportSummaryLine(
                            text = stringResource(
                                R.string.report_summary_error_category,
                                summary.errorCategory
                            )
                        )
                    }
                    if (summary.hints.isNotEmpty()) {
                        QueryReportSummaryLine(
                            text = stringResource(
                                R.string.report_summary_hints,
                                summary.hints.joinToString(separator = " | ")
                            )
                        )
                    }
                }

                is ReportSummary.WindowMetadata -> {
                    val periodLabel = stringResource(summary.period.reportModeResId())
                    val metadata = summary.metadata
                    Text(
                        text = stringResource(
                            R.string.report_result_title_report_window_summary,
                            periodLabel
                        ),
                        style = MaterialTheme.typography.titleMedium,
                        color = MaterialTheme.colorScheme.primary
                    )
                    Spacer(modifier = Modifier.height(8.dp))
                    Text(
                        text = if (metadata.hasRecords) {
                            stringResource(
                                R.string.report_summary_window_has_records_body,
                                periodLabel
                            )
                        } else {
                            stringResource(
                                R.string.report_summary_window_empty_body,
                                periodLabel
                            )
                        },
                        style = MaterialTheme.typography.bodyMedium
                    )
                    if (metadata.startDate.isNotBlank() || metadata.endDate.isNotBlank()) {
                        QueryReportSummaryLine(
                            text = stringResource(
                                R.string.report_summary_window_range,
                                metadata.startDate.ifBlank { "-" },
                                metadata.endDate.ifBlank { "-" }
                            )
                        )
                    }
                    if (metadata.requestedDays > 0) {
                        QueryReportSummaryLine(
                            text = stringResource(
                                R.string.report_summary_requested_days,
                                metadata.requestedDays
                            )
                        )
                    }
                    QueryReportSummaryLine(
                        text = stringResource(
                            R.string.report_summary_matched_days,
                            metadata.matchedDayCount
                        )
                    )
                    QueryReportSummaryLine(
                        text = stringResource(
                            R.string.report_summary_matched_records,
                            metadata.matchedRecordCount
                        )
                    )
                }
            }
        }
    }
}

@Composable
private fun QueryReportSummaryLine(text: String) {
    Spacer(modifier = Modifier.height(6.dp))
    Text(
        text = text,
        style = MaterialTheme.typography.bodySmall,
        modifier = Modifier.fillMaxWidth()
    )
}

internal fun DataTreePeriod.reportModeResId(): Int {
    return when (this) {
        DataTreePeriod.DAY -> R.string.report_mode_day
        DataTreePeriod.WEEK -> R.string.report_mode_week
        DataTreePeriod.MONTH -> R.string.report_mode_month
        DataTreePeriod.YEAR -> R.string.report_mode_year
        DataTreePeriod.RECENT -> R.string.report_mode_recent
        DataTreePeriod.RANGE -> R.string.report_mode_range
    }
}
