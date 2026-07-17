package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.aspectRatio
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.ArrowBack
import androidx.compose.material.icons.automirrored.filled.ArrowForward
import androidx.compose.material.icons.filled.ArrowDropDown
import androidx.compose.material.icons.filled.DateRange
import androidx.compose.material.icons.filled.Refresh
import androidx.compose.material3.Button
import androidx.compose.material3.ElevatedCard
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.record.R
import com.example.tracer.ui.components.CalendarAvailability
import com.example.tracer.ui.components.CalendarDatePickerSheet
import com.example.tracer.ui.components.CalendarYearMonthPickerSheet
import java.time.DayOfWeek
import java.time.LocalDate

// Match the visual footprint of the month row's Refresh action so the second
// day row keeps the same centered navigation geometry as the first row.
private val NavigationActionSlotWidth = 112.dp

@OptIn(ExperimentalMaterial3Api::class)
@Composable
internal fun TxtMonthNavigationCard(
    selectedMonth: String,
    currentDay: LocalDate?,
    onOpenPreviousMonth: () -> Unit,
    onOpenNextMonth: () -> Unit,
    onOpenPreviousDay: () -> Unit,
    onOpenNextDay: () -> Unit,
    onOpenDay: (LocalDate) -> Unit,
    onOpenMonth: (String) -> Unit,
    availableYears: List<String>,
    selectedYear: String,
    selectedMonthValue: String,
    monthsByYear: Map<String, List<String>>,
    onRefreshHistory: () -> Unit
) {
    var monthPickerVisible by remember { mutableStateOf(false) }
    var dayPickerVisible by remember { mutableStateOf(false) }

    ElevatedCard(modifier = Modifier.fillMaxWidth()) {
        val selectedMonthText = if (selectedMonth.isEmpty()) {
            stringResource(R.string.txt_label_select_file)
        } else {
            selectedMonth
        }
        val selectedDayText = if (currentDay != null) {
            formatCompactDayText(currentDay)
        } else {
            stringResource(R.string.txt_label_current_day_unselected)
        }
        val dayNavigationEnabled = currentDay != null

        Column(
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 8.dp, vertical = 6.dp),
            verticalArrangement = Arrangement.spacedBy(2.dp)
        ) {
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.spacedBy(4.dp),
                verticalAlignment = androidx.compose.ui.Alignment.CenterVertically
            ) {
                IconButton(onClick = onOpenPreviousMonth) {
                    Icon(
                        Icons.AutoMirrored.Filled.ArrowBack,
                        contentDescription = stringResource(R.string.txt_cd_prev_month)
                    )
                }

                TextButton(
                    onClick = {
                        monthPickerVisible = true
                    },
                    enabled = availableYears.isNotEmpty(),
                    modifier = Modifier.weight(1f)
                ) {
                    Text(
                        text = selectedMonthText,
                        maxLines = 1,
                        overflow = TextOverflow.Ellipsis,
                        style = MaterialTheme.typography.titleMedium,
                        color = MaterialTheme.colorScheme.primary
                    )
                    Spacer(modifier = Modifier.width(4.dp))
                    Icon(Icons.Filled.DateRange, contentDescription = null)
                }

                TextButton(onClick = onRefreshHistory) {
                    Icon(Icons.Filled.Refresh, contentDescription = null)
                    Spacer(modifier = Modifier.width(4.dp))
                    Text(stringResource(R.string.txt_action_refresh_list))
                }

                IconButton(onClick = onOpenNextMonth) {
                    Icon(
                        Icons.AutoMirrored.Filled.ArrowForward,
                        contentDescription = stringResource(R.string.txt_cd_next_month)
                    )
                }
            }
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.spacedBy(4.dp),
                verticalAlignment = androidx.compose.ui.Alignment.CenterVertically
            ) {
                IconButton(
                    onClick = onOpenPreviousDay,
                    enabled = dayNavigationEnabled
                ) {
                    Icon(
                        Icons.AutoMirrored.Filled.ArrowBack,
                        contentDescription = stringResource(R.string.txt_cd_prev_day)
                    )
                }

                TextButton(
                    onClick = { dayPickerVisible = true },
                    enabled = dayNavigationEnabled,
                    modifier = Modifier.weight(1f)
                ) {
                    Text(
                        text = selectedDayText,
                        style = MaterialTheme.typography.titleMedium,
                        color = MaterialTheme.colorScheme.primary,
                        maxLines = 1,
                        overflow = TextOverflow.Ellipsis
                    )
                    Spacer(modifier = Modifier.width(4.dp))
                    Icon(Icons.Filled.ArrowDropDown, contentDescription = null)
                }

                Spacer(
                    modifier = Modifier.width(NavigationActionSlotWidth)
                )

                IconButton(
                    onClick = onOpenNextDay,
                    enabled = dayNavigationEnabled
                ) {
                    Icon(
                        Icons.AutoMirrored.Filled.ArrowForward,
                        contentDescription = stringResource(R.string.txt_cd_next_day)
                    )
                }
            }
        }
    }

    if (monthPickerVisible) {
        CalendarYearMonthPickerSheet(
            selectedYearMonth = selectedMonth,
            availability = CalendarAvailability(monthsByYear),
            onYearMonthSelected = onOpenMonth,
            onDismissRequest = { monthPickerVisible = false },
            title = stringResource(R.string.txt_sheet_select_year_month),
            currentText = stringResource(
                R.string.txt_sheet_current_month,
                if (selectedMonth.isEmpty()) {
                    stringResource(R.string.txt_sheet_current_month_unselected)
                } else {
                    selectedMonth
                }
            ),
            yearTitle = stringResource(R.string.txt_picker_year_title),
            yearPlaceholder = stringResource(R.string.txt_picker_year_placeholder),
            noYearsLabel = stringResource(R.string.txt_picker_no_years)
        )
    }

    if (dayPickerVisible && currentDay != null) {
        CalendarDatePickerSheet(
            displayMonth = java.time.YearMonth.from(currentDay),
            selectedDate = currentDay,
            onDateSelected = onOpenDay,
            onDismissRequest = { dayPickerVisible = false },
            allowAdjacentMonthSelection = true,
            firstDayOfWeek = DayOfWeek.MONDAY
        )
    }
}

private fun formatCompactDayText(date: LocalDate): String =
    date.toString().substring(5)
