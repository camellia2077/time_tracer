package com.example.tracer

import androidx.compose.animation.AnimatedVisibility
import androidx.compose.animation.expandVertically
import androidx.compose.animation.shrinkVertically
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ExpandLess
import androidx.compose.material.icons.filled.ExpandMore
import androidx.compose.material3.ElevatedCard
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.report.R

@Composable
internal fun SegmentedAnalysisPeriodInputs(
    section: String,
    period: DataTreePeriod,
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
    onReportRecentDaysChange: (String) -> Unit
) {
    when (period) {
        DataTreePeriod.DAY -> {
            val (year, month, day) = splitDateDigits(reportDate)
            SegmentedDateInput(
                title = stringResource(R.string.report_title_section_day, section),
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

        DataTreePeriod.MONTH -> {
            val (year, month) = splitYearMonthDigits(reportMonth)
            SegmentedYearMonthInput(
                title = stringResource(R.string.report_title_section_month, section),
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

        DataTreePeriod.WEEK -> {
            val (year, week) = splitYearWeekDigits(reportWeek)
            SegmentedYearWeekInput(
                title = stringResource(R.string.report_title_section_week, section),
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

        DataTreePeriod.YEAR -> {
            OutlinedTextField(
                value = reportYear,
                onValueChange = onReportYearChange,
                label = { Text(stringResource(R.string.report_label_section_year, section)) },
                singleLine = true,
                keyboardOptions = keyboardOptions,
                modifier = Modifier.fillMaxWidth()
            )
        }

        DataTreePeriod.RANGE -> {
            val (startYear, startMonth, startDay) = splitDateDigits(reportRangeStartDate)
            SegmentedDateInput(
                title = stringResource(R.string.report_title_section_start_date, section),
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
                title = stringResource(R.string.report_title_section_end_date, section),
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

        DataTreePeriod.RECENT -> {
            OutlinedTextField(
                value = reportRecentDays,
                onValueChange = onReportRecentDaysChange,
                label = { Text(stringResource(R.string.report_label_section_recent_days, section)) },
                singleLine = true,
                keyboardOptions = keyboardOptions,
                modifier = Modifier.fillMaxWidth()
            )
        }
    }
}

@Composable
internal fun ExpandableParameterCard(
    title: String,
    expanded: Boolean,
    onToggle: () -> Unit,
    content: @Composable () -> Unit
) {
    ElevatedCard(
        modifier = Modifier.fillMaxWidth(),
        shape = MaterialTheme.shapes.extraLarge
    ) {
        Column(modifier = Modifier.fillMaxWidth()) {
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .clickable { onToggle() }
                    .padding(16.dp),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text(
                    text = title,
                    style = MaterialTheme.typography.titleMedium,
                    color = MaterialTheme.colorScheme.primary
                )
                Icon(
                    imageVector = if (expanded) Icons.Default.ExpandLess else Icons.Default.ExpandMore,
                    contentDescription = if (expanded) {
                        stringResource(R.string.report_cd_collapse)
                    } else {
                        stringResource(R.string.report_cd_expand)
                    },
                    tint = MaterialTheme.colorScheme.primary
                )
            }
            AnimatedVisibility(
                visible = expanded,
                enter = expandVertically(),
                exit = shrinkVertically()
            ) {
                Column(
                    modifier = Modifier
                        .padding(horizontal = 16.dp)
                        .padding(bottom = 16.dp),
                    verticalArrangement = Arrangement.spacedBy(12.dp)
                ) {
                    content()
                }
            }
        }
    }
}
