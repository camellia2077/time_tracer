package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.material3.DropdownMenu
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import com.example.tracer.ui.components.mergeYearMonthDigits
import com.example.tracer.ui.components.splitYearMonthDigits
import java.time.Year

@Composable
internal fun QueryYearMonthDropdownInput(
    title: String,
    reportMonth: String,
    onReportMonthChange: (String) -> Unit
) {
    val (rawYear, rawMonth) = splitYearMonthDigits(reportMonth)
    val selectedYear = rawYear.takeIf { it.length == 4 } ?: Year.now().value.toString()
    val selectedMonth = rawMonth.takeIf { it.length == 2 } ?: "01"
    val years = remember(selectedYear) {
        buildYearOptions(selectedYear)
    }
    val months = remember {
        (1..12).map { value -> value.toString().padStart(2, '0') }
    }

    var yearExpanded by remember { mutableStateOf(false) }
    var monthExpanded by remember { mutableStateOf(false) }

    Text(
        text = "$title (YYYY-MM)",
        style = MaterialTheme.typography.bodyMedium,
        color = MaterialTheme.colorScheme.onSurfaceVariant
    )
    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.spacedBy(8.dp)
    ) {
        OutlinedButton(
            onClick = { yearExpanded = true },
            modifier = Modifier.weight(1f)
        ) {
            Text(selectedYear)
        }
        DropdownMenu(
            expanded = yearExpanded,
            onDismissRequest = { yearExpanded = false }
        ) {
            years.forEach { year ->
                DropdownMenuItem(
                    text = { Text(year) },
                    onClick = {
                        onReportMonthChange(mergeYearMonthDigits(year, selectedMonth))
                        yearExpanded = false
                    }
                )
            }
        }

        OutlinedButton(
            onClick = { monthExpanded = true },
            modifier = Modifier.weight(1f)
        ) {
            Text(selectedMonth)
        }
        DropdownMenu(
            expanded = monthExpanded,
            onDismissRequest = { monthExpanded = false }
        ) {
            months.forEach { month ->
                DropdownMenuItem(
                    text = { Text(month) },
                    onClick = {
                        onReportMonthChange(mergeYearMonthDigits(selectedYear, month))
                        monthExpanded = false
                    }
                )
            }
        }
    }
}

private fun buildYearOptions(selectedYear: String): List<String> {
    val currentYear = Year.now().value
    val selectedYearInt = selectedYear.toIntOrNull() ?: currentYear
    val minYear = minOf(currentYear - 10, selectedYearInt - 2)
    val maxYear = maxOf(currentYear + 10, selectedYearInt + 2)
    return (minYear..maxYear).map { it.toString() }.reversed()
}

