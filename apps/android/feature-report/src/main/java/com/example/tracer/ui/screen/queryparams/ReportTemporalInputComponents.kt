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
