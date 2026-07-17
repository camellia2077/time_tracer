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
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ArrowDropDown
import androidx.compose.material3.DropdownMenu
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.ModalBottomSheet
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.Text
import androidx.compose.material3.rememberModalBottomSheetState
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp

data class CalendarAvailability(
    val monthsByYear: Map<String, List<String>>
) {
    // Report and TXT share the same data-backed calendar options so a user
    // cannot select a year/month for which no TXT month file is available.
    val years: List<String> = monthsByYear.keys.sorted()

    fun monthsForYear(year: String): Set<String> =
        monthsByYear[year].orEmpty().toSet()

    companion object {
        fun fromMonthKeys(monthKeys: Iterable<String>): CalendarAvailability {
            // Ignore malformed keys at the UI boundary; only canonical
            // YYYY-MM keys are valid choices for the shared picker.
            val monthsByYear = monthKeys
                .mapNotNull { key ->
                    val match = Regex("^(\\d{4})-(\\d{2})$").matchEntire(key)
                        ?: return@mapNotNull null
                    match.groupValues[1] to match.groupValues[2]
                }
                .groupBy({ it.first }, { it.second })
                .mapValues { (_, months) -> months.distinct().sorted() }
            return CalendarAvailability(monthsByYear)
        }
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun CalendarYearMonthPickerSheet(
    selectedYearMonth: String,
    availability: CalendarAvailability,
    onYearMonthSelected: (String) -> Unit,
    onDismissRequest: () -> Unit,
    title: String,
    currentText: String,
    yearTitle: String,
    yearPlaceholder: String,
    noYearsLabel: String
) {
    val sheetState = rememberModalBottomSheetState(skipPartiallyExpanded = true)
    val selectedYear = selectedYearMonth.takeIf { it.length >= 4 }?.take(4).orEmpty()
    val selectedMonth = selectedYearMonth.takeIf { it.length >= 7 }?.takeLast(2).orEmpty()
    var pickerYear by remember(selectedYear) { mutableStateOf(selectedYear) }
    var yearExpanded by remember { mutableStateOf(false) }
    val monthRows = remember {
        listOf(
            (1..6).map { it.toString().padStart(2, '0') },
            (7..12).map { it.toString().padStart(2, '0') }
        )
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
                text = title,
                style = MaterialTheme.typography.titleMedium,
                color = MaterialTheme.colorScheme.primary
            )
            Text(
                text = currentText,
                style = MaterialTheme.typography.bodySmall
            )

            Box(modifier = Modifier.fillMaxWidth()) {
                OutlinedButton(
                    onClick = { yearExpanded = true },
                    enabled = availability.years.isNotEmpty(),
                    modifier = Modifier.fillMaxWidth()
                ) {
                    Row(
                        modifier = Modifier.fillMaxWidth(),
                        horizontalArrangement = Arrangement.SpaceBetween,
                        verticalAlignment = Alignment.CenterVertically
                    ) {
                        Text(
                            text = "$yearTitle: ${pickerYear.ifBlank { yearPlaceholder }}",
                            maxLines = 1,
                            overflow = TextOverflow.Ellipsis
                        )
                        Icon(Icons.Filled.ArrowDropDown, contentDescription = null)
                    }
                }
                DropdownMenu(
                    expanded = yearExpanded,
                    onDismissRequest = { yearExpanded = false }
                ) {
                    if (availability.years.isEmpty()) {
                        DropdownMenuItem(
                            text = { Text(noYearsLabel) },
                            onClick = { yearExpanded = false }
                        )
                    } else {
                        availability.years.forEach { year ->
                            DropdownMenuItem(
                                text = { Text(year) },
                                onClick = {
                                    yearExpanded = false
                                    pickerYear = year
                                }
                            )
                        }
                    }
                }
            }

            val availableMonths = availability.monthsForYear(pickerYear)
            monthRows.forEach { rowMonths ->
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.spacedBy(8.dp)
                ) {
                    rowMonths.forEach { month ->
                        val enabled = availableMonths.contains(month)
                        val selected = pickerYear == selectedYear && month == selectedMonth
                        Box(
                            modifier = Modifier
                                .weight(1f)
                                .aspectRatio(1.6f)
                                .clip(RoundedCornerShape(12.dp))
                                .background(
                                    if (selected) MaterialTheme.colorScheme.primary
                                    else Color.Transparent
                                )
                                .then(
                                    if (enabled) {
                                        Modifier.clickable {
                                            onYearMonthSelected("$pickerYear-$month")
                                            onDismissRequest()
                                        }
                                    } else {
                                        Modifier
                                    }
                                ),
                            contentAlignment = Alignment.Center
                        ) {
                            Text(
                                text = month.toInt().toString(),
                                style = MaterialTheme.typography.titleSmall,
                                color = when {
                                    selected -> MaterialTheme.colorScheme.onPrimary
                                    enabled -> MaterialTheme.colorScheme.onSurface
                                    else -> MaterialTheme.colorScheme.onSurfaceVariant.copy(alpha = 0.75f)
                                },
                                textAlign = TextAlign.Center
                            )
                        }
                    }
                }
            }
            Spacer(modifier = Modifier.height(8.dp))
        }
    }
}
