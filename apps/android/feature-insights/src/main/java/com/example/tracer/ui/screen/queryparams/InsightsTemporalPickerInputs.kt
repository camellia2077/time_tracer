
package com.example.tracer

import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.fillMaxSize
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
import androidx.compose.ui.semantics.contentDescription
import androidx.compose.ui.semantics.semantics
import androidx.compose.ui.text.style.TextAlign
import com.example.tracer.feature.insights.R
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
@Composable
internal fun InsightsDayTemporalInput(
    labels: TemporalInputLabels,
    keyboardOptions: KeyboardOptions,
    insightsDate: String,
    calendarAvailability: CalendarAvailability,
    onInsightsDateChange: (String) -> Unit
) {
    val (year, month, day) = splitDateDigits(insightsDate)
    val dayPickerState = resolveInsightsDayPickerState(year, month, day)
    InsightsYearMonthPickerInput(
        title = labels.monthTitle,
        insightsMonth = mergeYearMonthDigits(year, month),
        availability = calendarAvailability,
        onInsightsMonthChange = { nextYearMonth ->
            val (nextYear, nextMonth) = splitYearMonthDigits(nextYearMonth)
            onInsightsDateChange(mergeDateDigits(nextYear, nextMonth, day))
        }
    )
    InsightsDayPickerInput(
        title = labels.dayTitle,
        day = day,
        keyboardOptions = keyboardOptions,
        dayPickerState = dayPickerState,
        onDayChange = { nextDay ->
            onInsightsDateChange(mergeDateDigits(year, month, nextDay))
        },
        onDayPicked = { pickedDate ->
            onInsightsDateChange(mergePickedInsightsDay(year, month, pickedDate))
        }
    )
}

@Composable
internal fun InsightsWeekTemporalInput(
    labels: TemporalInputLabels,
    insightsMonth: String,
    insightsWeek: String,
    calendarAvailability: CalendarAvailability,
    onInsightsMonthChange: (String) -> Unit,
    onInsightsWeekChange: (String) -> Unit
) {
    val weekPickerState = resolveInsightsWeekPickerState(insightsMonth, insightsWeek)
    InsightsYearMonthPickerInput(
        title = labels.monthTitle,
        insightsMonth = insightsMonth,
        availability = calendarAvailability,
        onInsightsMonthChange = onInsightsMonthChange
    )
    InsightsWeekPickerInput(
        title = labels.weekTitle,
        selectedWeekLabel = weekPickerState?.selectedWeekLabel,
        displayMonth = weekPickerState?.displayMonth,
        selectedWeekDigits = weekPickerState?.selectedWeekRow?.isoWeekDigits,
        onWeekPicked = onInsightsWeekChange
    )
}

@Composable
internal fun InsightsMonthTemporalInput(
    title: String,
    insightsMonth: String,
    calendarAvailability: CalendarAvailability,
    yearMonthValueFormat: InsightsYearMonthValueFormat = InsightsYearMonthValueFormat.COMPACT,
    onInsightsMonthChange: (String) -> Unit
) {
    InsightsYearMonthPickerInput(
        title = title,
        insightsMonth = insightsMonth,
        availability = calendarAvailability,
        valueFormat = yearMonthValueFormat,
        onInsightsMonthChange = onInsightsMonthChange
    )
}

@Composable
internal fun InsightsYearTemporalInput(
    title: String,
    insightsYear: String,
    calendarAvailability: CalendarAvailability,
    onInsightsYearChange: (String) -> Unit
) {
    InsightsYearPickerInput(
        title = title,
        insightsYear = insightsYear,
        availability = calendarAvailability,
        onInsightsYearChange = onInsightsYearChange
    )
}

@Composable
internal fun InsightsDayPickerInput(
    title: String,
    day: String,
    keyboardOptions: KeyboardOptions,
    dayPickerState: InsightsDayPickerState?,
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
internal fun InsightsWeekPickerInput(
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
    val weekPickerLabel = stringResource(R.string.insights_label_select_week)
    val openWeekPicker = { weekPickerVisible = true }
    Box(modifier = Modifier.fillMaxWidth()) {
        OutlinedTextField(
            value = selectedWeekLabel.orEmpty(),
            onValueChange = {},
            readOnly = true,
            placeholder = {
                Text(
                    text = weekPickerLabel,
                    modifier = Modifier.fillMaxWidth(),
                    textAlign = TextAlign.Start,
                    style = MaterialTheme.typography.bodySmall
                )
            },
            trailingIcon = {
                IconButton(
                    onClick = openWeekPicker,
                    enabled = displayMonth != null
                ) {
                    Icon(
                        imageVector = Icons.Filled.DateRange,
                        contentDescription = weekPickerLabel
                    )
                }
            },
            singleLine = true,
            textStyle = MaterialTheme.typography.titleMedium,
            shape = TracerOutlinedTextFieldDefaults.shape,
            modifier = Modifier.fillMaxWidth()
        )
        // A read-only TextField does not expose a click callback. The transparent overlay keeps
        // the whole date field, including its calendar icon, as one tappable picker trigger.
        Box(
            modifier = Modifier
                .fillMaxSize()
                .clickable(
                    enabled = displayMonth != null,
                    onClick = openWeekPicker
                )
                .semantics { contentDescription = weekPickerLabel }
        )
    }

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
