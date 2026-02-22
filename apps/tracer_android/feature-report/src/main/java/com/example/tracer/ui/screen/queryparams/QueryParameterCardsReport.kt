package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.List
import androidx.compose.material.icons.filled.DateRange
import androidx.compose.material.icons.filled.PlayArrow
import androidx.compose.material3.Button
import androidx.compose.material3.Icon
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.report.R

@Composable
internal fun ReportParametersCard(
    reportMode: ReportMode,
    expanded: Boolean,
    keyboardOptions: KeyboardOptions,
    reportDate: String,
    onReportDateChange: (String) -> Unit,
    reportMonth: String,
    onReportMonthChange: (String) -> Unit,
    reportYear: String,
    onReportYearChange: (String) -> Unit,
    reportWeek: String,
    onReportWeekChange: (String) -> Unit,
    reportRangeStartDate: String,
    onReportRangeStartDateChange: (String) -> Unit,
    reportRangeEndDate: String,
    onReportRangeEndDateChange: (String) -> Unit,
    reportRecentDays: String,
    onReportRecentDaysChange: (String) -> Unit,
    onToggle: () -> Unit,
    onRunReport: () -> Unit
) {
    ExpandableParameterCard(
        title = stringResource(
            R.string.report_title_mode_parameters,
            stringResource(reportMode.labelRes())
        ),
        expanded = expanded,
        onToggle = onToggle
    ) {
        Column(verticalArrangement = Arrangement.spacedBy(12.dp)) {
            when (reportMode) {
                ReportMode.DAY -> {
                    val (year, month, day) = splitDateDigits(reportDate)
                    SegmentedDateInput(
                        title = stringResource(R.string.report_title_report_date),
                        year = year,
                        month = month,
                        day = day,
                        keyboardOptions = keyboardOptions,
                        onYearChange = { nextYear ->
                            onReportDateChange(mergeDateDigits(nextYear, month, day))
                        },
                        onMonthChange = { nextMonth ->
                            onReportDateChange(mergeDateDigits(year, nextMonth, day))
                        },
                        onDayChange = { nextDay ->
                            onReportDateChange(mergeDateDigits(year, month, nextDay))
                        }
                    )
                }

                ReportMode.MONTH -> {
                    val (year, month) = splitYearMonthDigits(reportMonth)
                    SegmentedYearMonthInput(
                        title = stringResource(R.string.report_title_report_month),
                        year = year,
                        month = month,
                        keyboardOptions = keyboardOptions,
                        onYearChange = { nextYear ->
                            onReportMonthChange(mergeYearMonthDigits(nextYear, month))
                        },
                        onMonthChange = { nextMonth ->
                            onReportMonthChange(mergeYearMonthDigits(year, nextMonth))
                        }
                    )
                }

                ReportMode.WEEK -> {
                    val (year, week) = splitYearWeekDigits(reportWeek)
                    SegmentedYearWeekInput(
                        title = stringResource(R.string.report_title_report_week),
                        year = year,
                        week = week,
                        keyboardOptions = keyboardOptions,
                        onYearChange = { nextYear ->
                            onReportWeekChange(mergeYearWeekDigits(nextYear, week))
                        },
                        onWeekChange = { nextWeek ->
                            onReportWeekChange(mergeYearWeekDigits(year, nextWeek))
                        }
                    )
                }

                ReportMode.YEAR -> {
                    OutlinedTextField(
                        value = reportYear,
                        onValueChange = onReportYearChange,
                        label = { Text(stringResource(R.string.report_label_report_year)) },
                        leadingIcon = { Icon(Icons.Filled.DateRange, contentDescription = null) },
                        singleLine = true,
                        keyboardOptions = keyboardOptions,
                        modifier = Modifier.fillMaxWidth()
                    )
                }

                ReportMode.RANGE -> {
                    val (startYear, startMonth, startDay) = splitDateDigits(reportRangeStartDate)
                    SegmentedDateInput(
                        title = stringResource(R.string.report_title_start_date),
                        year = startYear,
                        month = startMonth,
                        day = startDay,
                        keyboardOptions = keyboardOptions,
                        onYearChange = { nextYear ->
                            onReportRangeStartDateChange(
                                mergeDateDigits(nextYear, startMonth, startDay)
                            )
                        },
                        onMonthChange = { nextMonth ->
                            onReportRangeStartDateChange(
                                mergeDateDigits(startYear, nextMonth, startDay)
                            )
                        },
                        onDayChange = { nextDay ->
                            onReportRangeStartDateChange(
                                mergeDateDigits(startYear, startMonth, nextDay)
                            )
                        }
                    )

                    val (endYear, endMonth, endDay) = splitDateDigits(reportRangeEndDate)
                    SegmentedDateInput(
                        title = stringResource(R.string.report_title_end_date),
                        year = endYear,
                        month = endMonth,
                        day = endDay,
                        keyboardOptions = keyboardOptions,
                        onYearChange = { nextYear ->
                            onReportRangeEndDateChange(mergeDateDigits(nextYear, endMonth, endDay))
                        },
                        onMonthChange = { nextMonth ->
                            onReportRangeEndDateChange(mergeDateDigits(endYear, nextMonth, endDay))
                        },
                        onDayChange = { nextDay ->
                            onReportRangeEndDateChange(mergeDateDigits(endYear, endMonth, nextDay))
                        }
                    )
                }

                ReportMode.RECENT -> {
                    OutlinedTextField(
                        value = reportRecentDays,
                        onValueChange = onReportRecentDaysChange,
                        label = { Text(stringResource(R.string.report_label_recent_days)) },
                        leadingIcon = { Icon(Icons.AutoMirrored.Filled.List, contentDescription = null) },
                        singleLine = true,
                        keyboardOptions = keyboardOptions,
                        modifier = Modifier.fillMaxWidth()
                    )
                }
            }

            Button(
                onClick = onRunReport,
                modifier = Modifier.fillMaxWidth()
            ) {
                Icon(Icons.Filled.PlayArrow, contentDescription = null)
                Spacer(modifier = Modifier.width(8.dp))
                Text(stringResource(R.string.report_action_generate_report_md))
            }
        }
    }
}
