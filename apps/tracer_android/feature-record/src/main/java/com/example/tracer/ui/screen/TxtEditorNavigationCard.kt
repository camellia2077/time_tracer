package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.ArrowBack
import androidx.compose.material.icons.automirrored.filled.ArrowForward
import androidx.compose.material.icons.filled.ArrowDropDown
import androidx.compose.material.icons.filled.Refresh
import androidx.compose.material3.Button
import androidx.compose.material3.DropdownMenu
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.ElevatedCard
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.ModalBottomSheet
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.material3.rememberModalBottomSheetState
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.record.R

@OptIn(ExperimentalMaterial3Api::class)
@Composable
internal fun TxtMonthNavigationCard(
    selectedMonth: String,
    onOpenPreviousMonth: () -> Unit,
    onOpenNextMonth: () -> Unit,
    onOpenMonth: (String) -> Unit,
    availableYears: List<String>,
    selectedYear: String,
    selectedMonthValue: String,
    monthsByYear: Map<String, List<String>>,
    onRefreshHistory: () -> Unit
) {
    var monthPickerVisible by remember { mutableStateOf(false) }
    var yearMenuExpanded by remember { mutableStateOf(false) }
    var monthMenuExpanded by remember { mutableStateOf(false) }
    var pickerYear by remember { mutableStateOf("") }
    var pickerMonth by remember { mutableStateOf("") }

    fun resetPickerSelection() {
        val initialYear = selectedYear.ifBlank { availableYears.lastOrNull().orEmpty() }
        val monthsForYear = monthsByYear[initialYear].orEmpty()
        val initialMonth = when {
            selectedMonthValue in monthsForYear -> selectedMonthValue
            else -> monthsForYear.lastOrNull().orEmpty()
        }
        pickerYear = initialYear
        pickerMonth = initialMonth
    }

    ElevatedCard(modifier = Modifier.fillMaxWidth()) {
        val selectedMonthText = if (selectedMonth.isEmpty()) {
            stringResource(R.string.txt_label_select_file)
        } else {
            selectedMonth
        }

        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 8.dp, vertical = 6.dp),
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
                    resetPickerSelection()
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
                Icon(Icons.Filled.ArrowDropDown, contentDescription = null)
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
    }

    if (monthPickerVisible) {
        val sheetState = rememberModalBottomSheetState(skipPartiallyExpanded = true)
        val monthChoices = monthsByYear[pickerYear].orEmpty()

        ModalBottomSheet(
            onDismissRequest = {
                monthPickerVisible = false
                yearMenuExpanded = false
                monthMenuExpanded = false
            },
            sheetState = sheetState
        ) {
            Column(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(horizontal = 16.dp, vertical = 8.dp),
                verticalArrangement = Arrangement.spacedBy(12.dp)
            ) {
                val currentMonthText = if (selectedMonth.isEmpty()) {
                    stringResource(R.string.txt_sheet_current_month_unselected)
                } else {
                    selectedMonth
                }

                Text(
                    text = stringResource(R.string.txt_sheet_select_year_month),
                    style = MaterialTheme.typography.titleMedium,
                    color = MaterialTheme.colorScheme.primary
                )
                Text(
                    text = stringResource(R.string.txt_sheet_current_month, currentMonthText),
                    style = MaterialTheme.typography.bodySmall
                )

                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.spacedBy(8.dp)
                ) {
                    MonthPickerDropdown(
                        title = stringResource(R.string.txt_picker_year_title),
                        value = pickerYear,
                        placeholder = stringResource(R.string.txt_picker_year_placeholder),
                        enabled = availableYears.isNotEmpty(),
                        expanded = yearMenuExpanded,
                        onExpandedChange = { yearMenuExpanded = it },
                        modifier = Modifier.weight(1f)
                    ) {
                        if (availableYears.isEmpty()) {
                            DropdownMenuItem(
                                text = { Text(stringResource(R.string.txt_picker_no_years)) },
                                onClick = { yearMenuExpanded = false }
                            )
                        } else {
                            availableYears.forEach { year ->
                                DropdownMenuItem(
                                    text = { Text(year) },
                                    onClick = {
                                        yearMenuExpanded = false
                                        pickerYear = year
                                        pickerMonth = monthsByYear[year].orEmpty().lastOrNull().orEmpty()
                                    }
                                )
                            }
                        }
                    }

                    MonthPickerDropdown(
                        title = stringResource(R.string.txt_picker_month_title),
                        value = pickerMonth,
                        placeholder = stringResource(R.string.txt_picker_month_placeholder),
                        enabled = pickerYear.isNotBlank() && monthChoices.isNotEmpty(),
                        expanded = monthMenuExpanded,
                        onExpandedChange = { monthMenuExpanded = it },
                        modifier = Modifier.weight(1f)
                    ) {
                        if (monthChoices.isEmpty()) {
                            DropdownMenuItem(
                                text = { Text(stringResource(R.string.txt_picker_no_months)) },
                                onClick = { monthMenuExpanded = false }
                            )
                        } else {
                            monthChoices.forEach { month ->
                                DropdownMenuItem(
                                    text = { Text(month) },
                                    onClick = {
                                        monthMenuExpanded = false
                                        pickerMonth = month
                                    }
                                )
                            }
                        }
                    }
                }

                Button(
                    onClick = {
                        val monthKey = if (pickerYear.isNotBlank() && pickerMonth.isNotBlank()) {
                            "$pickerYear-$pickerMonth"
                        } else {
                            ""
                        }
                        if (monthKey.isNotBlank()) {
                            onOpenMonth(monthKey)
                            monthPickerVisible = false
                            yearMenuExpanded = false
                            monthMenuExpanded = false
                        }
                    },
                    enabled = pickerYear.isNotBlank() && pickerMonth.isNotBlank(),
                    modifier = Modifier.fillMaxWidth()
                ) {
                    Text(stringResource(R.string.txt_picker_open_year_month, pickerYear, pickerMonth))
                }

                Spacer(modifier = Modifier.height(8.dp))
            }
        }
    }
}

@Composable
private fun MonthPickerDropdown(
    title: String,
    value: String,
    placeholder: String,
    enabled: Boolean,
    expanded: Boolean,
    onExpandedChange: (Boolean) -> Unit,
    modifier: Modifier = Modifier,
    menuContent: @Composable () -> Unit
) {
    Box(modifier = modifier) {
        OutlinedButton(
            onClick = { onExpandedChange(true) },
            enabled = enabled,
            modifier = Modifier.fillMaxWidth()
        ) {
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = androidx.compose.ui.Alignment.CenterVertically
            ) {
                Text(
                    text = stringResource(R.string.txt_picker_field_value, title, value.ifBlank { placeholder }),
                    maxLines = 1,
                    overflow = TextOverflow.Ellipsis
                )
                Icon(Icons.Filled.ArrowDropDown, contentDescription = null)
            }
        }

        DropdownMenu(
            expanded = expanded,
            onDismissRequest = { onExpandedChange(false) }
        ) {
            menuContent()
        }
    }
}
