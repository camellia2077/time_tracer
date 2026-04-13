package com.example.tracer.ui.components

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.aspectRatio
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.ModalBottomSheet
import androidx.compose.material3.Text
import androidx.compose.material3.rememberModalBottomSheetState
import androidx.compose.runtime.Composable
import androidx.compose.runtime.remember
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.uicommon.R
import java.time.DayOfWeek
import java.time.LocalDate
import java.time.YearMonth

data class CalendarDayCell(
    val date: LocalDate,
    val isInCurrentMonth: Boolean,
    val isSelected: Boolean,
    val isEnabled: Boolean
)

data class MonthCalendarGrid(
    val month: YearMonth,
    val cells: List<CalendarDayCell>
)

fun buildMonthCalendarGrid(
    displayMonth: YearMonth,
    selectedDate: LocalDate? = null,
    allowAdjacentMonthSelection: Boolean = true,
    firstDayOfWeek: DayOfWeek = DayOfWeek.MONDAY
): MonthCalendarGrid {
    val firstOfMonth = displayMonth.atDay(1)
    val lastOfMonth = displayMonth.atEndOfMonth()
    val leadingDays = dayOffset(
        day = firstOfMonth.dayOfWeek,
        firstDayOfWeek = firstDayOfWeek
    )
    val trailingDays = 6 - dayOffset(
        day = lastOfMonth.dayOfWeek,
        firstDayOfWeek = firstDayOfWeek
    )
    val startDate = firstOfMonth.minusDays(leadingDays.toLong())
    val endDate = lastOfMonth.plusDays(trailingDays.toLong())
    val cells = generateSequence(startDate) { current ->
        if (current >= endDate) {
            null
        } else {
            current.plusDays(1)
        }
    }.map { date ->
        val isInCurrentMonth = YearMonth.from(date) == displayMonth
        CalendarDayCell(
            date = date,
            isInCurrentMonth = isInCurrentMonth,
            isSelected = date == selectedDate,
            isEnabled = isInCurrentMonth || allowAdjacentMonthSelection
        )
    }.toList()
    return MonthCalendarGrid(month = displayMonth, cells = cells)
}

fun orderedWeekDays(
    firstDayOfWeek: DayOfWeek = DayOfWeek.MONDAY
): List<DayOfWeek> = List(size = 7) { index ->
    firstDayOfWeek.plus(index.toLong())
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun CalendarDatePickerSheet(
    displayMonth: YearMonth,
    selectedDate: LocalDate? = null,
    onDateSelected: (LocalDate) -> Unit,
    onDismissRequest: () -> Unit,
    allowAdjacentMonthSelection: Boolean = true,
    firstDayOfWeek: DayOfWeek = DayOfWeek.MONDAY
) {
    val sheetState = rememberModalBottomSheetState(skipPartiallyExpanded = true)
    val calendarGrid = remember(
        displayMonth,
        selectedDate,
        allowAdjacentMonthSelection,
        firstDayOfWeek
    ) {
        buildMonthCalendarGrid(
            displayMonth = displayMonth,
            selectedDate = selectedDate,
            allowAdjacentMonthSelection = allowAdjacentMonthSelection,
            firstDayOfWeek = firstDayOfWeek
        )
    }
    val weekDays = remember(firstDayOfWeek) {
        orderedWeekDays(firstDayOfWeek = firstDayOfWeek)
    }

    ModalBottomSheet(
        onDismissRequest = onDismissRequest,
        sheetState = sheetState
    ) {
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 16.dp, vertical = 8.dp),
            verticalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            Text(
                text = calendarGrid.month.toString(),
                style = MaterialTheme.typography.titleMedium,
                color = MaterialTheme.colorScheme.primary,
                textAlign = TextAlign.Center,
                modifier = Modifier.fillMaxWidth()
            )

            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.spacedBy(4.dp)
            ) {
                weekDays.forEach { dayOfWeek ->
                    CalendarWeekdayHeader(
                        dayOfWeek = dayOfWeek,
                        modifier = Modifier.weight(1f)
                    )
                }
            }

            calendarGrid.cells.chunked(7).forEach { week ->
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.spacedBy(4.dp)
                ) {
                    week.forEach { cell ->
                        CalendarDayCellButton(
                            cell = cell,
                            modifier = Modifier.weight(1f),
                            onClick = {
                                if (cell.isEnabled) {
                                    onDateSelected(cell.date)
                                    onDismissRequest()
                                }
                            }
                        )
                    }
                }
            }

            Spacer(modifier = Modifier.height(8.dp))
        }
    }
}

@Composable
private fun CalendarWeekdayHeader(
    dayOfWeek: DayOfWeek,
    modifier: Modifier = Modifier
) {
    Box(
        modifier = modifier.padding(vertical = 4.dp),
        contentAlignment = Alignment.Center
    ) {
        Text(
            text = stringResource(dayOfWeek.shortWeekdayLabelRes()),
            style = MaterialTheme.typography.titleSmall,
            color = MaterialTheme.colorScheme.onSurfaceVariant
        )
    }
}

@Composable
private fun CalendarDayCellButton(
    cell: CalendarDayCell,
    modifier: Modifier = Modifier,
    onClick: () -> Unit
) {
    val backgroundColor = if (cell.isSelected) {
        MaterialTheme.colorScheme.primary
    } else {
        Color.Transparent
    }
    val textColor = when {
        cell.isSelected -> MaterialTheme.colorScheme.onPrimary
        cell.isInCurrentMonth -> MaterialTheme.colorScheme.onSurface
        else -> MaterialTheme.colorScheme.onSurfaceVariant.copy(alpha = 0.6f)
    }
    val clickModifier = if (cell.isEnabled) {
        Modifier.clickable(onClick = onClick)
    } else {
        Modifier
    }

    Box(
        modifier = modifier
            .aspectRatio(1f)
            .clip(RoundedCornerShape(12.dp))
            .background(backgroundColor)
            .then(clickModifier),
        contentAlignment = Alignment.Center
    ) {
        Text(
            text = cell.date.dayOfMonth.toString(),
            style = MaterialTheme.typography.titleSmall,
            color = if (cell.isEnabled) textColor else textColor.copy(alpha = 0.5f),
            textAlign = TextAlign.Center
        )
    }
}

private fun DayOfWeek.shortWeekdayLabelRes(): Int = when (this) {
    DayOfWeek.MONDAY -> R.string.calendar_weekday_short_mon
    DayOfWeek.TUESDAY -> R.string.calendar_weekday_short_tue
    DayOfWeek.WEDNESDAY -> R.string.calendar_weekday_short_wed
    DayOfWeek.THURSDAY -> R.string.calendar_weekday_short_thu
    DayOfWeek.FRIDAY -> R.string.calendar_weekday_short_fri
    DayOfWeek.SATURDAY -> R.string.calendar_weekday_short_sat
    DayOfWeek.SUNDAY -> R.string.calendar_weekday_short_sun
}

private fun dayOffset(
    day: DayOfWeek,
    firstDayOfWeek: DayOfWeek
): Int = (day.value - firstDayOfWeek.value + 7) % 7
