
package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ArrowDropDown
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.ModalBottomSheet
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
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.insights.R
import com.example.tracer.ui.components.CalendarDatePickerSheet
import com.example.tracer.ui.components.CalendarAvailability
import com.example.tracer.ui.components.CalendarWeekPickerSheet
import com.example.tracer.ui.components.OutlinedPickerField
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
    InsightsDateSelectionInput(
        title = labels.dayTitle,
        insightsDate = insightsDate,
        monthTitle = labels.monthTitle,
        keyboardOptions = keyboardOptions,
        calendarAvailability = calendarAvailability,
        onMonthChange = { nextYearMonth ->
            val (nextYear, nextMonth) = splitYearMonthDigits(nextYearMonth)
            onInsightsDateChange(mergeDateDigits(nextYear, nextMonth, day))
        },
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
    InsightsWeekPickerInput(
        title = labels.weekTitle,
        monthTitle = labels.monthTitle,
        insightsMonth = insightsMonth,
        calendarAvailability = calendarAvailability,
        selectedWeekLabel = weekPickerState?.selectedWeekLabel,
        displayMonth = weekPickerState?.displayMonth,
        selectedWeekDigits = weekPickerState?.selectedWeekRow?.isoWeekDigits,
        onInsightsMonthChange = onInsightsMonthChange,
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

@OptIn(ExperimentalMaterial3Api::class)
@Composable
internal fun InsightsDateSelectionInput(
    title: String,
    insightsDate: String,
    monthTitle: String,
    keyboardOptions: KeyboardOptions,
    calendarAvailability: CalendarAvailability,
    onMonthChange: (String) -> Unit,
    onDayChange: (String) -> Unit,
    onDayPicked: (java.time.LocalDate) -> Unit
) {
    var selectionSheetVisible by remember { mutableStateOf(false) }
    val (year, month, selectedDay) = splitDateDigits(insightsDate)
    val dayPickerState = resolveInsightsDayPickerState(insightsDate)
    val datePickerLabel = stringResource(
        com.example.tracer.feature.uicommon.R.string.calendar_cd_select_day
    )
    val displayDate = dayPickerState?.selectedDate?.toString() ?: insightsDate
    val openSelectionSheet = { selectionSheetVisible = true }

    OutlinedPickerField(
        title = title,
        value = displayDate,
        enabled = dayPickerState != null,
        pickerContentDescription = datePickerLabel,
        onOpen = openSelectionSheet
    )

    if (selectionSheetVisible) {
        ModalBottomSheet(
            onDismissRequest = { selectionSheetVisible = false }
        ) {
            Column(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(horizontal = 16.dp, vertical = 8.dp),
                verticalArrangement = Arrangement.spacedBy(12.dp)
            ) {
                InsightsYearMonthPickerInput(
                    title = monthTitle,
                    insightsMonth = mergeYearMonthDigits(year, month),
                    availability = calendarAvailability,
                    onInsightsMonthChange = onMonthChange
                )
                InsightsDayPickerInput(
                    title = title,
                    day = selectedDay,
                    keyboardOptions = keyboardOptions,
                    dayPickerState = dayPickerState,
                    onDayChange = onDayChange,
                    onDayPicked = onDayPicked
                )
            }
        }
    }
}

@Composable
private fun InsightsDayPickerInput(
    title: String,
    day: String,
    keyboardOptions: KeyboardOptions,
    dayPickerState: InsightsDayPickerState?,
    onDayChange: (String) -> Unit,
    onDayPicked: (java.time.LocalDate) -> Unit
) {
    var dayPickerVisible by remember { mutableStateOf(false) }

    Text(
        text = title,
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
                        imageVector = Icons.Filled.ArrowDropDown,
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

@OptIn(ExperimentalMaterial3Api::class)
@Composable
internal fun InsightsWeekPickerInput(
    title: String,
    monthTitle: String,
    insightsMonth: String,
    calendarAvailability: CalendarAvailability,
    selectedWeekLabel: String?,
    displayMonth: java.time.YearMonth?,
    selectedWeekDigits: String?,
    onInsightsMonthChange: (String) -> Unit,
    onWeekPicked: (String) -> Unit
) {
    var selectionSheetVisible by remember { mutableStateOf(false) }
    var calendarVisible by remember { mutableStateOf(false) }

    val weekPickerLabel = stringResource(R.string.insights_label_select_week)
    val openSelectionSheet = { selectionSheetVisible = true }
    val openCalendar = { calendarVisible = true }
    InsightsWeekPickerField(
        title = title,
        selectedWeekLabel = selectedWeekLabel,
        weekPickerLabel = weekPickerLabel,
        enabled = displayMonth != null,
        onOpen = openSelectionSheet
    )

    if (selectionSheetVisible) {
        ModalBottomSheet(
            onDismissRequest = { selectionSheetVisible = false }
        ) {
            Column(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(horizontal = 16.dp, vertical = 8.dp),
                verticalArrangement = Arrangement.spacedBy(12.dp)
            ) {
                InsightsYearMonthPickerInput(
                    title = monthTitle,
                    insightsMonth = insightsMonth,
                    availability = calendarAvailability,
                    onInsightsMonthChange = onInsightsMonthChange
                )
                InsightsWeekPickerField(
                    title = title,
                    selectedWeekLabel = selectedWeekLabel,
                    weekPickerLabel = weekPickerLabel,
                    enabled = displayMonth != null,
                    onOpen = openCalendar
                )
            }
        }
    }

    if (calendarVisible && displayMonth != null) {
        CalendarWeekPickerSheet(
            displayMonth = displayMonth,
            selectedWeekDigits = selectedWeekDigits,
            onWeekSelected = onWeekPicked,
            onDismissRequest = { calendarVisible = false },
            firstDayOfWeek = DayOfWeek.MONDAY
        )
    }
}

@Composable
private fun InsightsWeekPickerField(
    title: String,
    selectedWeekLabel: String?,
    weekPickerLabel: String,
    enabled: Boolean,
    onOpen: () -> Unit
) {
    OutlinedPickerField(
        title = title,
        value = selectedWeekLabel.orEmpty(),
        placeholder = weekPickerLabel,
        enabled = enabled,
        pickerContentDescription = weekPickerLabel,
        onOpen = onOpen
    )
}
