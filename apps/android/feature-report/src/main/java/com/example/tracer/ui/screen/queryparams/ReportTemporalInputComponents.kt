package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.DateRange
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.style.TextAlign
import com.example.tracer.feature.report.R
import com.example.tracer.ui.components.CalendarDatePickerSheet
import com.example.tracer.ui.components.CalendarAvailability
import com.example.tracer.ui.components.CalendarWeekPickerSheet
import com.example.tracer.ui.components.SegmentedDateInput
import com.example.tracer.ui.components.TracerOutlinedTextFieldDefaults
import com.example.tracer.ui.components.filterDigits
import com.example.tracer.ui.components.mergeDateDigits
import com.example.tracer.ui.components.mergeYearMonthDigits
import com.example.tracer.ui.components.splitDateDigits
import com.example.tracer.ui.components.splitYearMonthDigits
import java.time.DayOfWeek

internal data class TemporalInputLabels(
    val dayTitle: String,
    val monthTitle: String,
    val weekTitle: String,
    val yearLabel: String,
    val rangeStartTitle: String,
    val rangeEndTitle: String,
    val recentDaysLabel: String
)

@Composable
internal fun ReportTemporalInputFields(
    period: DataTreePeriod,
    labels: TemporalInputLabels,
    keyboardOptions: KeyboardOptions,
    reportDate: String,
    onReportDateChange: (String) -> Unit,
    reportMonth: String,
    onReportMonthChange: (String) -> Unit,
    calendarAvailability: CalendarAvailability,
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
        DataTreePeriod.DAY -> ReportDayTemporalInput(
            labels = labels,
            keyboardOptions = keyboardOptions,
            reportDate = reportDate,
            calendarAvailability = calendarAvailability,
            onReportDateChange = onReportDateChange
        )

        DataTreePeriod.WEEK -> ReportWeekTemporalInput(
            labels = labels,
            reportMonth = reportMonth,
            reportWeek = reportWeek,
            calendarAvailability = calendarAvailability,
            onReportMonthChange = onReportMonthChange,
            onReportWeekChange = onReportWeekChange
        )

        DataTreePeriod.MONTH -> ReportMonthTemporalInput(
            title = labels.monthTitle,
            reportMonth = reportMonth,
            calendarAvailability = calendarAvailability,
            onReportMonthChange = onReportMonthChange
        )

        DataTreePeriod.YEAR -> ReportYearTemporalInput(
            title = labels.yearLabel,
            reportYear = reportYear,
            calendarAvailability = calendarAvailability,
            onReportYearChange = onReportYearChange
        )

        DataTreePeriod.RANGE -> {
            val (startYear, startMonth, startDay) = splitDateDigits(reportRangeStartDate)
            SegmentedDateInput(
                title = labels.rangeStartTitle,
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
                title = labels.rangeEndTitle,
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
                label = { Text(labels.recentDaysLabel) },
                singleLine = true,
                keyboardOptions = keyboardOptions,
                shape = TracerOutlinedTextFieldDefaults.shape,
                modifier = Modifier.fillMaxWidth()
            )
        }
    }
}

@Composable
private fun ReportDayTemporalInput(
    labels: TemporalInputLabels,
    keyboardOptions: KeyboardOptions,
    reportDate: String,
    calendarAvailability: CalendarAvailability,
    onReportDateChange: (String) -> Unit
) {
    val (year, month, day) = splitDateDigits(reportDate)
    val dayPickerState = resolveReportDayPickerState(year, month, day)
    ReportYearMonthPickerInput(
        title = labels.monthTitle,
        reportMonth = mergeYearMonthDigits(year, month),
        availability = calendarAvailability,
        onReportMonthChange = { nextYearMonth ->
            val (nextYear, nextMonth) = splitYearMonthDigits(nextYearMonth)
            onReportDateChange(mergeDateDigits(nextYear, nextMonth, day))
        }
    )
    ReportDayPickerInput(
        title = labels.dayTitle,
        day = day,
        keyboardOptions = keyboardOptions,
        dayPickerState = dayPickerState,
        onDayChange = { nextDay ->
            onReportDateChange(mergeDateDigits(year, month, nextDay))
        },
        onDayPicked = { pickedDate ->
            onReportDateChange(mergePickedReportDay(year, month, pickedDate))
        }
    )
}

@Composable
private fun ReportWeekTemporalInput(
    labels: TemporalInputLabels,
    reportMonth: String,
    reportWeek: String,
    calendarAvailability: CalendarAvailability,
    onReportMonthChange: (String) -> Unit,
    onReportWeekChange: (String) -> Unit
) {
    val weekPickerState = resolveReportWeekPickerState(reportMonth, reportWeek)
    ReportYearMonthPickerInput(
        title = labels.monthTitle,
        reportMonth = reportMonth,
        availability = calendarAvailability,
        onReportMonthChange = onReportMonthChange
    )
    ReportWeekPickerInput(
        title = labels.weekTitle,
        selectedWeekLabel = weekPickerState?.selectedWeekLabel,
        displayMonth = weekPickerState?.displayMonth,
        selectedWeekDigits = weekPickerState?.selectedWeekRow?.isoWeekDigits,
        onWeekPicked = onReportWeekChange
    )
}

@Composable
private fun ReportMonthTemporalInput(
    title: String,
    reportMonth: String,
    calendarAvailability: CalendarAvailability,
    onReportMonthChange: (String) -> Unit
) {
    ReportYearMonthPickerInput(
        title = title,
        reportMonth = reportMonth,
        availability = calendarAvailability,
        onReportMonthChange = onReportMonthChange
    )
}

@Composable
private fun ReportYearTemporalInput(
    title: String,
    reportYear: String,
    calendarAvailability: CalendarAvailability,
    onReportYearChange: (String) -> Unit
) {
    ReportYearPickerInput(
        title = title,
        reportYear = reportYear,
        availability = calendarAvailability,
        onReportYearChange = onReportYearChange
    )
}

@Composable
internal fun ReportDayPickerInput(
    title: String,
    day: String,
    keyboardOptions: KeyboardOptions,
    dayPickerState: ReportDayPickerState?,
    onDayChange: (String) -> Unit,
    onDayPicked: (java.time.LocalDate) -> Unit
) {
    var dayPickerVisible by remember { mutableStateOf(false) }

    Text(
        text = "$title (DD)",
        style = MaterialTheme.typography.bodyMedium,
        color = MaterialTheme.colorScheme.onSurfaceVariant
    )
    Row(modifier = Modifier.fillMaxWidth()) {
        OutlinedTextField(
            value = day,
            onValueChange = { onDayChange(filterDigits(it, 2)) },
            placeholder = {
                Text(
                    text = "DD",
                    modifier = Modifier.fillMaxWidth(),
                    textAlign = TextAlign.Center,
                    style = MaterialTheme.typography.bodySmall
                )
            },
            singleLine = true,
            keyboardOptions = keyboardOptions,
            textStyle = MaterialTheme.typography.titleMedium.copy(textAlign = TextAlign.Center),
            shape = TracerOutlinedTextFieldDefaults.shape,
            trailingIcon = {
                IconButton(
                    onClick = { dayPickerVisible = true },
                    enabled = dayPickerState != null
                ) {
                    Icon(
                        imageVector = Icons.Filled.DateRange,
                        contentDescription = stringResource(
                            com.example.tracer.feature.uicommon.R.string.calendar_cd_select_day
                        )
                    )
                }
            },
            modifier = Modifier.fillMaxWidth()
        )
    }

    if (dayPickerVisible && dayPickerState != null) {
        CalendarDatePickerSheet(
            displayMonth = dayPickerState.displayMonth,
            selectedDate = dayPickerState.selectedDate,
            onDateSelected = onDayPicked,
            onDismissRequest = { dayPickerVisible = false },
            allowAdjacentMonthSelection = false,
            firstDayOfWeek = DayOfWeek.MONDAY
        )
    }
}

@Composable
internal fun ReportWeekPickerInput(
    title: String,
    selectedWeekLabel: String?,
    displayMonth: java.time.YearMonth?,
    selectedWeekDigits: String?,
    onWeekPicked: (String) -> Unit
) {
    var weekPickerVisible by remember { mutableStateOf(false) }

    Text(
        text = title,
        style = MaterialTheme.typography.bodyMedium,
        color = MaterialTheme.colorScheme.onSurfaceVariant
    )
    OutlinedTextField(
        value = selectedWeekLabel.orEmpty(),
        onValueChange = {},
        readOnly = true,
        placeholder = {
            Text(
                text = stringResource(R.string.report_label_select_week),
                modifier = Modifier.fillMaxWidth(),
                textAlign = TextAlign.Start,
                style = MaterialTheme.typography.bodySmall
            )
        },
        trailingIcon = {
            IconButton(
                onClick = { weekPickerVisible = true },
                enabled = displayMonth != null
            ) {
                Icon(
                    imageVector = Icons.Filled.DateRange,
                    contentDescription = stringResource(R.string.report_label_select_week)
                )
            }
        },
        singleLine = true,
        textStyle = MaterialTheme.typography.titleMedium,
        shape = TracerOutlinedTextFieldDefaults.shape,
        modifier = Modifier.fillMaxWidth()
    )

    if (weekPickerVisible && displayMonth != null) {
        CalendarWeekPickerSheet(
            displayMonth = displayMonth,
            selectedWeekDigits = selectedWeekDigits,
            onWeekSelected = onWeekPicked,
            onDismissRequest = { weekPickerVisible = false },
            firstDayOfWeek = DayOfWeek.MONDAY
        )
    }
}
