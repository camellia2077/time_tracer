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

@Composable
internal fun QueryYearMonthDropdownInput(
    title: String,
    reportMonth: String,
    availableYears: List<String>,
    onReportMonthChange: (String) -> Unit
) {
    val (rawYear, rawMonth) = splitYearMonthDigits(reportMonth)
    val selectedYear = rawYear.takeIf { it.length == 4 }
        ?: availableYears.firstOrNull()
        ?: ""
    val selectedMonth = rawMonth.takeIf { it.length == 2 } ?: "01"
    val years = remember(availableYears, selectedYear) {
        if (selectedYear.isBlank()) {
            availableYears
        } else {
            (listOf(selectedYear) + availableYears).distinct()
        }
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
            enabled = years.isNotEmpty(),
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
