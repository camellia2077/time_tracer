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
import com.example.tracer.feature.insights.R
import com.example.tracer.ui.components.CalendarDatePickerSheet
import com.example.tracer.ui.components.CalendarAvailability
import com.example.tracer.ui.components.CalendarWeekPickerSheet
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
internal fun InsightsTemporalInputFields(
    period: DataTreePeriod,
    labels: TemporalInputLabels,
    keyboardOptions: KeyboardOptions,
    insightsDate: String,
    onInsightsDateChange: (String) -> Unit,
    insightsMonth: String,
    onInsightsMonthChange: (String) -> Unit,
    yearMonthValueFormat: InsightsYearMonthValueFormat = InsightsYearMonthValueFormat.COMPACT,
    calendarAvailability: CalendarAvailability,
    insightsYear: String,
    onInsightsYearChange: (String) -> Unit,
    insightsWeek: String,
    onInsightsWeekChange: (String) -> Unit,
    insightsRangeStartDate: String,
    onInsightsRangeStartDateChange: (String) -> Unit,
    insightsRangeEndDate: String,
    onInsightsRangeEndDateChange: (String) -> Unit,
    insightsRecentDays: String,
    onInsightsRecentDaysChange: (String) -> Unit
) {
    when (period) {
        DataTreePeriod.DAY -> InsightsDayTemporalInput(
            labels = labels,
            keyboardOptions = keyboardOptions,
            insightsDate = insightsDate,
            calendarAvailability = calendarAvailability,
            onInsightsDateChange = onInsightsDateChange
        )

        DataTreePeriod.WEEK -> InsightsWeekTemporalInput(
            labels = labels,
            insightsMonth = insightsMonth,
            insightsWeek = insightsWeek,
            calendarAvailability = calendarAvailability,
            onInsightsMonthChange = onInsightsMonthChange,
            onInsightsWeekChange = onInsightsWeekChange
        )

        DataTreePeriod.MONTH -> InsightsMonthTemporalInput(
            title = labels.monthTitle,
            insightsMonth = insightsMonth,
            calendarAvailability = calendarAvailability,
            yearMonthValueFormat = yearMonthValueFormat,
            onInsightsMonthChange = onInsightsMonthChange
        )

        DataTreePeriod.YEAR -> InsightsYearTemporalInput(
            title = labels.yearLabel,
            insightsYear = insightsYear,
            calendarAvailability = calendarAvailability,
            onInsightsYearChange = onInsightsYearChange
        )

        DataTreePeriod.RANGE -> {
            val (startYear, startMonth, startDay) = splitDateDigits(insightsRangeStartDate)
            InsightsDateSelectionInput(
                title = labels.rangeStartTitle,
                insightsDate = insightsRangeStartDate,
                monthTitle = labels.monthTitle,
                keyboardOptions = keyboardOptions,
                calendarAvailability = calendarAvailability,
                onMonthChange = { nextYearMonth ->
                    val (nextYear, nextMonth) = splitYearMonthDigits(nextYearMonth)
                    onInsightsRangeStartDateChange(
                        mergeDateDigits(nextYear, nextMonth, startDay)
                    )
                },
                onDayChange = { nextDay ->
                    onInsightsRangeStartDateChange(
                        mergeDateDigits(startYear, startMonth, nextDay)
                    )
                },
                onDayPicked = { pickedDate ->
                    onInsightsRangeStartDateChange(
                        mergePickedInsightsDay(startYear, startMonth, pickedDate)
                    )
                }
            )

            val (endYear, endMonth, endDay) = splitDateDigits(insightsRangeEndDate)
            InsightsDateSelectionInput(
                title = labels.rangeEndTitle,
                insightsDate = insightsRangeEndDate,
                monthTitle = labels.monthTitle,
                keyboardOptions = keyboardOptions,
                calendarAvailability = calendarAvailability,
                onMonthChange = { nextYearMonth ->
                    val (nextYear, nextMonth) = splitYearMonthDigits(nextYearMonth)
                    onInsightsRangeEndDateChange(
                        mergeDateDigits(nextYear, nextMonth, endDay)
                    )
                },
                onDayChange = { nextDay ->
                    onInsightsRangeEndDateChange(mergeDateDigits(endYear, endMonth, nextDay))
                },
                onDayPicked = { pickedDate ->
                    onInsightsRangeEndDateChange(
                        mergePickedInsightsDay(endYear, endMonth, pickedDate)
                    )
                }
            )
        }

        DataTreePeriod.RECENT -> {
            OutlinedTextField(
                value = insightsRecentDays,
                onValueChange = onInsightsRecentDaysChange,
                label = { Text(labels.recentDaysLabel) },
                singleLine = true,
                keyboardOptions = keyboardOptions,
                shape = TracerOutlinedTextFieldDefaults.shape,
                modifier = Modifier.fillMaxWidth()
            )
        }
    }
}
