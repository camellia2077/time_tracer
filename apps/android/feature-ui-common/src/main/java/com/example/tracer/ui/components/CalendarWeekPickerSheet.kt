package com.example.tracer.ui.components

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
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
import java.time.format.DateTimeFormatter
import java.time.temporal.WeekFields
import java.util.Locale

data class CalendarWeekDayCell(
    val date: LocalDate,
    val isInCurrentMonth: Boolean
)

data class CalendarWeekRow(
    val weekStart: LocalDate,
    val weekEnd: LocalDate,
    val isoWeekDigits: String,
    val days: List<CalendarWeekDayCell>,
    val isSelected: Boolean
)

// The sheet is month-scoped for user recognition, but every row still resolves
// to a canonical ISO week based on that row's Monday so report queries remain
// aligned with ISO week semantics.
fun buildMonthWeekRows(
    displayMonth: YearMonth,
    selectedWeekDigits: String? = null,
    firstDayOfWeek: DayOfWeek = DayOfWeek.MONDAY
): List<CalendarWeekRow> {
    val firstOfMonth = displayMonth.atDay(1)
    val lastOfMonth = displayMonth.atEndOfMonth()
    val leadingDays = weekDayOffset(
        day = firstOfMonth.dayOfWeek,
        firstDayOfWeek = firstDayOfWeek
    )
    val trailingDays = 6 - weekDayOffset(
        day = lastOfMonth.dayOfWeek,
        firstDayOfWeek = firstDayOfWeek
    )
    val startDate = firstOfMonth.minusDays(leadingDays.toLong())
    val endDate = lastOfMonth.plusDays(trailingDays.toLong())
    val allDates = generateSequence(startDate) { current ->
        if (current >= endDate) {
            null
        } else {
            current.plusDays(1)
        }
    }.toList()

    return allDates.chunked(7).map { weekDates ->
        val weekStart = weekDates.first()
        val weekEnd = weekDates.last()
        val isoAnchor = weekDates.firstOrNull { it.dayOfWeek == DayOfWeek.MONDAY }
            ?: weekStart
        val isoWeekDigits = isoWeekDigits(isoAnchor)
        CalendarWeekRow(
            weekStart = weekStart,
            weekEnd = weekEnd,
            isoWeekDigits = isoWeekDigits,
            days = weekDates.map { date ->
                CalendarWeekDayCell(
                    date = date,
                    isInCurrentMonth = YearMonth.from(date) == displayMonth
                )
            },
            isSelected = isoWeekDigits == selectedWeekDigits
        )
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun CalendarWeekPickerSheet(
    displayMonth: YearMonth,
    selectedWeekDigits: String? = null,
    onWeekSelected: (String) -> Unit,
    onDismissRequest: () -> Unit,
    firstDayOfWeek: DayOfWeek = DayOfWeek.MONDAY
) {
    val sheetState = rememberModalBottomSheetState(skipPartiallyExpanded = true)
    val weekRows = remember(displayMonth, selectedWeekDigits, firstDayOfWeek) {
        buildMonthWeekRows(
            displayMonth = displayMonth,
            selectedWeekDigits = selectedWeekDigits,
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
                text = displayMonth.toString(),
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
                    WeekdayHeader(
                        dayOfWeek = dayOfWeek,
                        modifier = Modifier.weight(1f)
                    )
                }
            }

            weekRows.forEach { weekRow ->
                CalendarWeekRowButton(
                    weekRow = weekRow,
                    onClick = {
                        onWeekSelected(weekRow.isoWeekDigits)
                        onDismissRequest()
                    }
                )
            }

            Spacer(modifier = Modifier.height(8.dp))
        }
    }
}

@Composable
private fun WeekdayHeader(
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
private fun CalendarWeekRowButton(
    weekRow: CalendarWeekRow,
    onClick: () -> Unit
) {
    val containerColor = if (weekRow.isSelected) {
        MaterialTheme.colorScheme.primary
    } else {
        MaterialTheme.colorScheme.surfaceVariant.copy(alpha = 0.45f)
    }
    val titleColor = if (weekRow.isSelected) {
        MaterialTheme.colorScheme.onPrimary
    } else {
        MaterialTheme.colorScheme.onSurface
    }
    val weekColor = if (weekRow.isSelected) {
        MaterialTheme.colorScheme.onPrimary.copy(alpha = 0.85f)
    } else {
        MaterialTheme.colorScheme.onSurfaceVariant
    }

    Column(
        modifier = Modifier
            .fillMaxWidth()
            .clip(RoundedCornerShape(16.dp))
            .background(containerColor)
            .clickable(onClick = onClick)
            .padding(horizontal = 12.dp, vertical = 10.dp),
        verticalArrangement = Arrangement.spacedBy(8.dp)
    ) {
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.SpaceBetween,
            verticalAlignment = Alignment.CenterVertically
        ) {
            Text(
                text = formatWeekRangeText(weekRow.weekStart, weekRow.weekEnd),
                style = MaterialTheme.typography.titleSmall,
                color = titleColor
            )
            Text(
                text = "W${weekRow.isoWeekDigits.takeLast(2)}",
                style = MaterialTheme.typography.labelLarge,
                color = weekColor
            )
        }

        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.spacedBy(4.dp)
        ) {
            weekRow.days.forEach { day ->
                WeekDayCell(
                    day = day,
                    selected = weekRow.isSelected,
                    modifier = Modifier.weight(1f)
                )
            }
        }
    }
}

@Composable
private fun WeekDayCell(
    day: CalendarWeekDayCell,
    selected: Boolean,
    modifier: Modifier = Modifier
) {
    val textColor = when {
        selected && day.isInCurrentMonth -> MaterialTheme.colorScheme.onPrimary
        selected -> MaterialTheme.colorScheme.onPrimary.copy(alpha = 0.7f)
        day.isInCurrentMonth -> MaterialTheme.colorScheme.onSurface
        else -> MaterialTheme.colorScheme.onSurfaceVariant.copy(alpha = 0.6f)
    }
    Box(
        modifier = modifier.padding(vertical = 2.dp),
        contentAlignment = Alignment.Center
    ) {
        Text(
            text = day.date.dayOfMonth.toString(),
            style = MaterialTheme.typography.titleSmall,
            color = textColor,
            textAlign = TextAlign.Center
        )
    }
}

fun formatWeekRangeText(
    weekStart: LocalDate,
    weekEnd: LocalDate
): String {
    val formatter = DateTimeFormatter.ofPattern("MM-dd", Locale.US)
    return "${weekStart.format(formatter)} ~ ${weekEnd.format(formatter)}"
}

private fun isoWeekDigits(weekStart: LocalDate): String {
    val weekFields = WeekFields.ISO
    val weekYear = weekStart.get(weekFields.weekBasedYear())
    val week = weekStart.get(weekFields.weekOfWeekBasedYear())
    return String.format(Locale.US, "%04d%02d", weekYear, week)
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

private fun weekDayOffset(
    day: DayOfWeek,
    firstDayOfWeek: DayOfWeek
): Int = (day.value - firstDayOfWeek.value + 7) % 7
