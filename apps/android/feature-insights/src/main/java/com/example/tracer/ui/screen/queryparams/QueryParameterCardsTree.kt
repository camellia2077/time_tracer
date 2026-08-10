package com.example.tracer

import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.material3.DropdownMenu
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.ExposedDropdownMenuAnchorType
import androidx.compose.material3.ExposedDropdownMenuBox
import androidx.compose.material3.ExposedDropdownMenuDefaults
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import com.example.tracer.feature.insights.R
import com.example.tracer.ui.components.TracerOutlinedTextFieldDefaults

@Composable
internal fun TreeParametersCard(
    analysisPeriod: DataTreePeriod,
    maxAvailableDepth: Int,
    treeLevel: String,
    keyboardOptions: KeyboardOptions,
    insightsDate: String,
    onInsightsDateChange: (String) -> Unit,
    insightsMonth: String,
    onInsightsMonthChange: (String) -> Unit,
    calendarAvailability: com.example.tracer.ui.components.CalendarAvailability,
    insightsYear: String,
    onInsightsYearChange: (String) -> Unit,
    insightsWeek: String,
    onInsightsWeekChange: (String) -> Unit,
    insightsRangeStartDate: String,
    onInsightsRangeStartDateChange: (String) -> Unit,
    insightsRangeEndDate: String,
    onInsightsRangeEndDateChange: (String) -> Unit,
    insightsRecentDays: String,
    onInsightsRecentDaysChange: (String) -> Unit,
    onTreeLevelChange: (String) -> Unit,
    expanded: Boolean,
    onExpandedChange: (Boolean) -> Unit
) {
    TemporalParametersCard(
        title = stringResource(
            R.string.insights_title_mode_parameters,
            stringResource(R.string.insights_section_tree)
        ),
        period = analysisPeriod,
        maxAvailableDepth = maxAvailableDepth,
        keyboardOptions = keyboardOptions,
        insightsDate = insightsDate,
        onInsightsDateChange = onInsightsDateChange,
        insightsMonth = insightsMonth,
        onInsightsMonthChange = onInsightsMonthChange,
        calendarAvailability = calendarAvailability,
        insightsYear = insightsYear,
        onInsightsYearChange = onInsightsYearChange,
        insightsWeek = insightsWeek,
        onInsightsWeekChange = onInsightsWeekChange,
        insightsRangeStartDate = insightsRangeStartDate,
        onInsightsRangeStartDateChange = onInsightsRangeStartDateChange,
        insightsRangeEndDate = insightsRangeEndDate,
        onInsightsRangeEndDateChange = onInsightsRangeEndDateChange,
        insightsRecentDays = insightsRecentDays,
        onInsightsRecentDaysChange = onInsightsRecentDaysChange,
        expanded = expanded,
        onExpandedChange = onExpandedChange,
        treeLevel = treeLevel,
        onTreeLevelChange = onTreeLevelChange
    )
}

@Composable
@OptIn(ExperimentalMaterial3Api::class)
internal fun TreeLevelDropdown(
    treeLevel: String,
    keyboardOptions: KeyboardOptions,
    maxAvailableDepth: Int,
    onTreeLevelChange: (String) -> Unit
) {
    var expanded by remember { mutableStateOf(false) }
    val selectedLevel = treeLevel.toIntOrNull() ?: -1
    val levelOptions = remember(selectedLevel, maxAvailableDepth) {
        (listOf(-1) + (0..maxAvailableDepth.coerceAtLeast(0)).toList() + selectedLevel)
            .distinct()
    }
    val selectedLabel = if (selectedLevel == -1) {
        stringResource(R.string.insights_tree_level_all)
    } else {
        stringResource(R.string.insights_tree_level_number, selectedLevel)
    }

    ExposedDropdownMenuBox(
        expanded = expanded,
        onExpandedChange = { expanded = !expanded }
    ) {
        OutlinedTextField(
            value = selectedLabel,
            onValueChange = {},
            readOnly = true,
            label = { Text(stringResource(R.string.insights_label_tree_level)) },
            trailingIcon = {
                ExposedDropdownMenuDefaults.TrailingIcon(expanded = expanded)
            },
            keyboardOptions = keyboardOptions,
            shape = TracerOutlinedTextFieldDefaults.shape,
            modifier = Modifier
                .fillMaxWidth()
                .menuAnchor(ExposedDropdownMenuAnchorType.PrimaryNotEditable)
        )
        DropdownMenu(
            expanded = expanded,
            onDismissRequest = { expanded = false }
        ) {
            levelOptions.forEach { level ->
                val label = if (level == -1) {
                    stringResource(R.string.insights_tree_level_all)
                } else {
                    stringResource(R.string.insights_tree_level_number, level)
                }
                DropdownMenuItem(
                    text = { Text(label) },
                    onClick = {
                        expanded = false
                        onTreeLevelChange(level.toString())
                    }
                )
            }
        }
    }
}
