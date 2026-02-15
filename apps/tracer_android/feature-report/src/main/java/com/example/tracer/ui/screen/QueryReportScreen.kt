package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.animation.AnimatedVisibility
import androidx.compose.animation.expandVertically
import androidx.compose.animation.shrinkVertically
import androidx.compose.foundation.clickable
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.List
import androidx.compose.material.icons.filled.DateRange
import androidx.compose.material.icons.filled.ExpandLess
import androidx.compose.material.icons.filled.ExpandMore
import androidx.compose.material.icons.filled.PlayArrow
import androidx.compose.material3.Button
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.ElevatedCard
import androidx.compose.material3.ExposedDropdownMenuAnchorType
import androidx.compose.material3.ExposedDropdownMenuDefaults
import androidx.compose.material3.ExposedDropdownMenuBox
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.PrimaryScrollableTabRow
import androidx.compose.material3.Tab
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.dp

private enum class ReportMode(val label: String) {
    DAY("Day"),
    MONTH("Month"),
    WEEK("Week"),
    YEAR("Year"),
    RANGE("Range"),
    RECENT("Recent")
}

private val SegmentYearWidth: Dp = 108.dp
private val SegmentShortWidth: Dp = 68.dp
private val SegmentFieldHeight: Dp = 52.dp
private val TreePeriodOptions: List<DataTreePeriod> = listOf(
    DataTreePeriod.DAY,
    DataTreePeriod.MONTH,
    DataTreePeriod.WEEK,
    DataTreePeriod.YEAR,
    DataTreePeriod.RECENT,
    DataTreePeriod.RANGE
)

@Composable
fun QueryReportSection(
    reportDate: String,
    onReportDateChange: (String) -> Unit,
    reportMonth: String,
    onReportMonthChange: (String) -> Unit,
    reportYear: String,
    onReportYearChange: (String) -> Unit,
    reportWeek: String,
    onReportWeekChange: (String) -> Unit,
    reportRangeStartDate: String,
    onReportRangeStartDateChange: (String) -> Unit,
    reportRangeEndDate: String,
    onReportRangeEndDateChange: (String) -> Unit,
    reportRecentDays: String,
    onReportRecentDaysChange: (String) -> Unit,
    onReportDay: () -> Unit,
    onReportMonth: () -> Unit,
    onReportYear: () -> Unit,
    onReportWeek: () -> Unit,
    onReportRange: () -> Unit,
    onReportRecent: () -> Unit,
    analysisLoading: Boolean,
    onLoadDayStats: (DataTreePeriod) -> Unit,
    onLoadTree: (DataTreePeriod, Int) -> Unit
) {
    var reportMode by rememberSaveable { mutableStateOf(ReportMode.DAY) }
    var statsPeriod by rememberSaveable { mutableStateOf(DataTreePeriod.RECENT) }
    var treePeriod by rememberSaveable { mutableStateOf(DataTreePeriod.RECENT) }
    var treeLevel by rememberSaveable { mutableStateOf("-1") }

    var reportExpanded by rememberSaveable { mutableStateOf(true) }
    var statsExpanded by rememberSaveable { mutableStateOf(false) }
    var treeExpanded by rememberSaveable { mutableStateOf(false) }
    val reportModes = ReportMode.entries
    val selectedIndex = reportModes.indexOf(reportMode)
    val numericKeyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Number)

    Column(
        modifier = Modifier.fillMaxWidth(),
        verticalArrangement = Arrangement.spacedBy(16.dp)
    ) {
        PrimaryScrollableTabRow(
            selectedTabIndex = selectedIndex,
            modifier = Modifier.fillMaxWidth(),
            edgePadding = 0.dp
        ) {
            reportModes.forEach { mode ->
                Tab(
                    selected = reportMode == mode,
                    onClick = { reportMode = mode },
                    text = {
                        Text(
                            text = mode.label,
                            maxLines = 1,
                            softWrap = false
                        )
                    }
                )
            }
        }

        ExpandableParameterCard(
            title = "${reportMode.label} Parameters",
            expanded = reportExpanded,
            onToggle = { reportExpanded = !reportExpanded }
        ) {
            Column(
                verticalArrangement = Arrangement.spacedBy(12.dp)
            ) {
                when (reportMode) {
                    ReportMode.DAY -> {
                        val (year, month, day) = splitDateDigits(reportDate)
                        SegmentedDateInput(
                            title = "Report Date",
                            year = year,
                            month = month,
                            day = day,
                            keyboardOptions = numericKeyboardOptions,
                            onYearChange = { nextYear ->
                                onReportDateChange(mergeDateDigits(nextYear, month, day))
                            },
                            onMonthChange = { nextMonth ->
                                onReportDateChange(mergeDateDigits(year, nextMonth, day))
                            },
                            onDayChange = { nextDay ->
                                onReportDateChange(mergeDateDigits(year, month, nextDay))
                            }
                        )
                    }

                    ReportMode.MONTH -> {
                        val (year, month) = splitYearMonthDigits(reportMonth)
                        SegmentedYearMonthInput(
                            title = "Report Month",
                            year = year,
                            month = month,
                            keyboardOptions = numericKeyboardOptions,
                            onYearChange = { nextYear ->
                                onReportMonthChange(mergeYearMonthDigits(nextYear, month))
                            },
                            onMonthChange = { nextMonth ->
                                onReportMonthChange(mergeYearMonthDigits(year, nextMonth))
                            }
                        )
                    }

                    ReportMode.WEEK -> {
                        val (year, week) = splitYearWeekDigits(reportWeek)
                        SegmentedYearWeekInput(
                            title = "Report Week",
                            year = year,
                            week = week,
                            keyboardOptions = numericKeyboardOptions,
                            onYearChange = { nextYear ->
                                onReportWeekChange(mergeYearWeekDigits(nextYear, week))
                            },
                            onWeekChange = { nextWeek ->
                                onReportWeekChange(mergeYearWeekDigits(year, nextWeek))
                            }
                        )
                    }

                    ReportMode.YEAR -> {
                        OutlinedTextField(
                            value = reportYear,
                            onValueChange = onReportYearChange,
                            label = { Text("Report Year (YYYY)") },
                            leadingIcon = { Icon(Icons.Filled.DateRange, contentDescription = null) },
                            singleLine = true,
                            keyboardOptions = numericKeyboardOptions,
                            modifier = Modifier.fillMaxWidth()
                        )
                    }

                    ReportMode.RANGE -> {
                        val (startYear, startMonth, startDay) = splitDateDigits(reportRangeStartDate)
                        SegmentedDateInput(
                            title = "Start Date",
                            year = startYear,
                            month = startMonth,
                            day = startDay,
                            keyboardOptions = numericKeyboardOptions,
                            onYearChange = { nextYear ->
                                onReportRangeStartDateChange(
                                    mergeDateDigits(nextYear, startMonth, startDay)
                                )
                            },
                            onMonthChange = { nextMonth ->
                                onReportRangeStartDateChange(
                                    mergeDateDigits(startYear, nextMonth, startDay)
                                )
                            },
                            onDayChange = { nextDay ->
                                onReportRangeStartDateChange(
                                    mergeDateDigits(startYear, startMonth, nextDay)
                                )
                            }
                        )

                        val (endYear, endMonth, endDay) = splitDateDigits(reportRangeEndDate)
                        SegmentedDateInput(
                            title = "End Date",
                            year = endYear,
                            month = endMonth,
                            day = endDay,
                            keyboardOptions = numericKeyboardOptions,
                            onYearChange = { nextYear ->
                                onReportRangeEndDateChange(mergeDateDigits(nextYear, endMonth, endDay))
                            },
                            onMonthChange = { nextMonth ->
                                onReportRangeEndDateChange(mergeDateDigits(endYear, nextMonth, endDay))
                            },
                            onDayChange = { nextDay ->
                                onReportRangeEndDateChange(mergeDateDigits(endYear, endMonth, nextDay))
                            }
                        )
                    }

                    ReportMode.RECENT -> {
                        OutlinedTextField(
                            value = reportRecentDays,
                            onValueChange = onReportRecentDaysChange,
                            label = { Text("Recent Days (N)") },
                            leadingIcon = { Icon(Icons.AutoMirrored.Filled.List, contentDescription = null) },
                            singleLine = true,
                            keyboardOptions = numericKeyboardOptions,
                            modifier = Modifier.fillMaxWidth()
                        )
                    }
                }

                Button(
                    onClick = {
                        when (reportMode) {
                            ReportMode.DAY -> onReportDay()
                            ReportMode.MONTH -> onReportMonth()
                            ReportMode.WEEK -> onReportWeek()
                            ReportMode.YEAR -> onReportYear()
                            ReportMode.RANGE -> onReportRange()
                            ReportMode.RECENT -> onReportRecent()
                        }
                        reportExpanded = false
                    },
                    modifier = Modifier.fillMaxWidth()
                ) {
                    Icon(Icons.Filled.PlayArrow, contentDescription = null)
                    Spacer(modifier = Modifier.width(8.dp))
                    Text("Generate Report MD")
                }
            }
        }

        ExpandableParameterCard(
            title = "Stats Parameters",
            expanded = statsExpanded,
            onToggle = { statsExpanded = !statsExpanded }
        ) {
            Column(
                verticalArrangement = Arrangement.spacedBy(12.dp)
            ) {
                TreePeriodDropdown(
                    selectedPeriod = statsPeriod,
                    label = "Stats Period",
                    onPeriodSelected = { next ->
                        statsPeriod = next
                    }
                )
                when (statsPeriod) {
                    DataTreePeriod.DAY -> {
                        val (year, month, day) = splitDateDigits(reportDate)
                        SegmentedDateInput(
                            title = "Stats Day",
                            year = year,
                            month = month,
                            day = day,
                            keyboardOptions = numericKeyboardOptions,
                            onYearChange = { nextYear ->
                                onReportDateChange(mergeDateDigits(nextYear, month, day))
                            },
                            onMonthChange = { nextMonth ->
                                onReportDateChange(mergeDateDigits(year, nextMonth, day))
                            },
                            onDayChange = { nextDay ->
                                onReportDateChange(mergeDateDigits(year, month, nextDay))
                            }
                        )
                    }

                    DataTreePeriod.MONTH -> {
                        val (year, month) = splitYearMonthDigits(reportMonth)
                        SegmentedYearMonthInput(
                            title = "Stats Month",
                            year = year,
                            month = month,
                            keyboardOptions = numericKeyboardOptions,
                            onYearChange = { nextYear ->
                                onReportMonthChange(mergeYearMonthDigits(nextYear, month))
                            },
                            onMonthChange = { nextMonth ->
                                onReportMonthChange(mergeYearMonthDigits(year, nextMonth))
                            }
                        )
                    }

                    DataTreePeriod.WEEK -> {
                        val (year, week) = splitYearWeekDigits(reportWeek)
                        SegmentedYearWeekInput(
                            title = "Stats Week",
                            year = year,
                            week = week,
                            keyboardOptions = numericKeyboardOptions,
                            onYearChange = { nextYear ->
                                onReportWeekChange(mergeYearWeekDigits(nextYear, week))
                            },
                            onWeekChange = { nextWeek ->
                                onReportWeekChange(mergeYearWeekDigits(year, nextWeek))
                            }
                        )
                    }

                    DataTreePeriod.YEAR -> {
                        OutlinedTextField(
                            value = reportYear,
                            onValueChange = onReportYearChange,
                            label = { Text("Stats Year (YYYY)") },
                            singleLine = true,
                            keyboardOptions = numericKeyboardOptions,
                            modifier = Modifier.fillMaxWidth()
                        )
                    }

                    DataTreePeriod.RANGE -> {
                        val (startYear, startMonth, startDay) = splitDateDigits(reportRangeStartDate)
                        SegmentedDateInput(
                            title = "Stats Start Date",
                            year = startYear,
                            month = startMonth,
                            day = startDay,
                            keyboardOptions = numericKeyboardOptions,
                            onYearChange = { nextYear ->
                                onReportRangeStartDateChange(
                                    mergeDateDigits(nextYear, startMonth, startDay)
                                )
                            },
                            onMonthChange = { nextMonth ->
                                onReportRangeStartDateChange(
                                    mergeDateDigits(startYear, nextMonth, startDay)
                                )
                            },
                            onDayChange = { nextDay ->
                                onReportRangeStartDateChange(
                                    mergeDateDigits(startYear, startMonth, nextDay)
                                )
                            }
                        )

                        val (endYear, endMonth, endDay) = splitDateDigits(reportRangeEndDate)
                        SegmentedDateInput(
                            title = "Stats End Date",
                            year = endYear,
                            month = endMonth,
                            day = endDay,
                            keyboardOptions = numericKeyboardOptions,
                            onYearChange = { nextYear ->
                                onReportRangeEndDateChange(mergeDateDigits(nextYear, endMonth, endDay))
                            },
                            onMonthChange = { nextMonth ->
                                onReportRangeEndDateChange(mergeDateDigits(endYear, nextMonth, endDay))
                            },
                            onDayChange = { nextDay ->
                                onReportRangeEndDateChange(mergeDateDigits(endYear, endMonth, nextDay))
                            }
                        )
                    }

                    DataTreePeriod.RECENT -> {
                        OutlinedTextField(
                            value = reportRecentDays,
                            onValueChange = onReportRecentDaysChange,
                            label = { Text("Stats Recent Days (N)") },
                            singleLine = true,
                            keyboardOptions = numericKeyboardOptions,
                            modifier = Modifier.fillMaxWidth()
                        )
                    }
                }
                Button(
                    onClick = {
                        onLoadDayStats(statsPeriod)
                        statsExpanded = false
                    },
                    enabled = !analysisLoading,
                    modifier = Modifier.fillMaxWidth()
                ) {
                    Text(if (analysisLoading) "Loading..." else "Generate ${statsPeriod.wireValue} Stats")
                }
            }
        }

        ExpandableParameterCard(
            title = "Project Tree Parameters",
            expanded = treeExpanded,
            onToggle = { treeExpanded = !treeExpanded }
        ) {
            Column(
                verticalArrangement = Arrangement.spacedBy(12.dp)
            ) {
                TreePeriodDropdown(
                    selectedPeriod = treePeriod,
                    label = "Tree Period",
                    onPeriodSelected = { next ->
                        treePeriod = next
                    }
                )
                when (treePeriod) {
                    DataTreePeriod.DAY -> {
                        val (year, month, day) = splitDateDigits(reportDate)
                        SegmentedDateInput(
                            title = "Tree Day",
                            year = year,
                            month = month,
                            day = day,
                            keyboardOptions = numericKeyboardOptions,
                            onYearChange = { nextYear ->
                                onReportDateChange(mergeDateDigits(nextYear, month, day))
                            },
                            onMonthChange = { nextMonth ->
                                onReportDateChange(mergeDateDigits(year, nextMonth, day))
                            },
                            onDayChange = { nextDay ->
                                onReportDateChange(mergeDateDigits(year, month, nextDay))
                            }
                        )
                    }

                    DataTreePeriod.MONTH -> {
                        val (year, month) = splitYearMonthDigits(reportMonth)
                        SegmentedYearMonthInput(
                            title = "Tree Month",
                            year = year,
                            month = month,
                            keyboardOptions = numericKeyboardOptions,
                            onYearChange = { nextYear ->
                                onReportMonthChange(mergeYearMonthDigits(nextYear, month))
                            },
                            onMonthChange = { nextMonth ->
                                onReportMonthChange(mergeYearMonthDigits(year, nextMonth))
                            }
                        )
                    }

                    DataTreePeriod.WEEK -> {
                        val (year, week) = splitYearWeekDigits(reportWeek)
                        SegmentedYearWeekInput(
                            title = "Tree Week",
                            year = year,
                            week = week,
                            keyboardOptions = numericKeyboardOptions,
                            onYearChange = { nextYear ->
                                onReportWeekChange(mergeYearWeekDigits(nextYear, week))
                            },
                            onWeekChange = { nextWeek ->
                                onReportWeekChange(mergeYearWeekDigits(year, nextWeek))
                            }
                        )
                    }

                    DataTreePeriod.YEAR -> {
                        OutlinedTextField(
                            value = reportYear,
                            onValueChange = onReportYearChange,
                            label = { Text("Tree Year (YYYY)") },
                            singleLine = true,
                            keyboardOptions = numericKeyboardOptions,
                            modifier = Modifier.fillMaxWidth()
                        )
                    }

                    DataTreePeriod.RANGE -> {
                        val (startYear, startMonth, startDay) = splitDateDigits(reportRangeStartDate)
                        SegmentedDateInput(
                            title = "Tree Start Date",
                            year = startYear,
                            month = startMonth,
                            day = startDay,
                            keyboardOptions = numericKeyboardOptions,
                            onYearChange = { nextYear ->
                                onReportRangeStartDateChange(
                                    mergeDateDigits(nextYear, startMonth, startDay)
                                )
                            },
                            onMonthChange = { nextMonth ->
                                onReportRangeStartDateChange(
                                    mergeDateDigits(startYear, nextMonth, startDay)
                                )
                            },
                            onDayChange = { nextDay ->
                                onReportRangeStartDateChange(
                                    mergeDateDigits(startYear, startMonth, nextDay)
                                )
                            }
                        )

                        val (endYear, endMonth, endDay) = splitDateDigits(reportRangeEndDate)
                        SegmentedDateInput(
                            title = "Tree End Date",
                            year = endYear,
                            month = endMonth,
                            day = endDay,
                            keyboardOptions = numericKeyboardOptions,
                            onYearChange = { nextYear ->
                                onReportRangeEndDateChange(mergeDateDigits(nextYear, endMonth, endDay))
                            },
                            onMonthChange = { nextMonth ->
                                onReportRangeEndDateChange(mergeDateDigits(endYear, nextMonth, endDay))
                            },
                            onDayChange = { nextDay ->
                                onReportRangeEndDateChange(mergeDateDigits(endYear, endMonth, nextDay))
                            }
                        )
                    }

                    DataTreePeriod.RECENT -> {
                        OutlinedTextField(
                            value = reportRecentDays,
                            onValueChange = onReportRecentDaysChange,
                            label = { Text("Tree Recent Days (N)") },
                            singleLine = true,
                            keyboardOptions = numericKeyboardOptions,
                            modifier = Modifier.fillMaxWidth()
                        )
                    }
                }
                OutlinedTextField(
                    value = treeLevel,
                    onValueChange = { treeLevel = normalizeSignedIntInput(it, 3) },
                    label = { Text("Tree Level (-1 means unlimited)") },
                    singleLine = true,
                    keyboardOptions = numericKeyboardOptions,
                    modifier = Modifier.fillMaxWidth()
                )
                Button(
                    onClick = {
                        val parsedLevel = treeLevel.trim().toIntOrNull() ?: -1
                        onLoadTree(treePeriod, parsedLevel)
                        treeExpanded = false
                    },
                    enabled = !analysisLoading,
                    modifier = Modifier.fillMaxWidth()
                ) {
                    Text(if (analysisLoading) "Loading..." else "Load Project Tree")
                }
            }
        }
    }
}

@Composable
private fun ExpandableParameterCard(
    title: String,
    expanded: Boolean,
    onToggle: () -> Unit,
    content: @Composable () -> Unit
) {
    ElevatedCard(
        modifier = Modifier.fillMaxWidth(),
        shape = MaterialTheme.shapes.extraLarge
    ) {
        Column(modifier = Modifier.fillMaxWidth()) {
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .clickable { onToggle() }
                    .padding(16.dp),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text(
                    text = title,
                    style = MaterialTheme.typography.titleMedium,
                    color = MaterialTheme.colorScheme.primary
                )
                Icon(
                    imageVector = if (expanded) Icons.Default.ExpandLess else Icons.Default.ExpandMore,
                    contentDescription = if (expanded) "Collapse" else "Expand",
                    tint = MaterialTheme.colorScheme.primary
                )
            }
            AnimatedVisibility(
                visible = expanded,
                enter = expandVertically(),
                exit = shrinkVertically()
            ) {
                Column(
                    modifier = Modifier
                        .padding(horizontal = 16.dp)
                        .padding(bottom = 16.dp),
                    verticalArrangement = Arrangement.spacedBy(12.dp)
                ) {
                    content()
                }
            }
        }
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
private fun TreePeriodDropdown(
    selectedPeriod: DataTreePeriod,
    label: String,
    onPeriodSelected: (DataTreePeriod) -> Unit
) {
    var expanded by rememberSaveable { mutableStateOf(false) }
    ExposedDropdownMenuBox(
        expanded = expanded,
        onExpandedChange = { expanded = !expanded },
        modifier = Modifier.fillMaxWidth()
    ) {
        OutlinedTextField(
            value = selectedPeriod.wireValue,
            onValueChange = {},
            readOnly = true,
            label = { Text(label) },
            trailingIcon = {
                ExposedDropdownMenuDefaults.TrailingIcon(expanded = expanded)
            },
            modifier = Modifier
                .menuAnchor(ExposedDropdownMenuAnchorType.PrimaryNotEditable)
                .fillMaxWidth()
        )
        ExposedDropdownMenu(
            expanded = expanded,
            onDismissRequest = { expanded = false }
        ) {
            TreePeriodOptions.forEach { period ->
                DropdownMenuItem(
                    text = { Text(period.wireValue) },
                    onClick = {
                        onPeriodSelected(period)
                        expanded = false
                    }
                )
            }
        }
    }
}

@Composable
private fun SegmentedDateInput(
    title: String,
    year: String,
    month: String,
    day: String,
    keyboardOptions: KeyboardOptions,
    onYearChange: (String) -> Unit,
    onMonthChange: (String) -> Unit,
    onDayChange: (String) -> Unit
) {
    Text(
        text = "$title (YYYY-MM-DD)",
        style = MaterialTheme.typography.bodyMedium,
        color = MaterialTheme.colorScheme.onSurfaceVariant
    )
    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.spacedBy(6.dp),
        verticalAlignment = Alignment.CenterVertically
    ) {
        SegmentNumberField(
            value = year,
            onValueChange = { onYearChange(filterDigits(it, 4)) },
            keyboardOptions = keyboardOptions,
            width = SegmentYearWidth,
            placeholder = "YYYY"
        )
        IsoSeparator("-")
        SegmentNumberField(
            value = month,
            onValueChange = { onMonthChange(filterDigits(it, 2)) },
            keyboardOptions = keyboardOptions,
            width = SegmentShortWidth,
            placeholder = "MM"
        )
        IsoSeparator("-")
        SegmentNumberField(
            value = day,
            onValueChange = { onDayChange(filterDigits(it, 2)) },
            keyboardOptions = keyboardOptions,
            width = SegmentShortWidth,
            placeholder = "DD"
        )
    }
}

@Composable
private fun SegmentedYearMonthInput(
    title: String,
    year: String,
    month: String,
    keyboardOptions: KeyboardOptions,
    onYearChange: (String) -> Unit,
    onMonthChange: (String) -> Unit
) {
    Text(
        text = "$title (YYYY-MM)",
        style = MaterialTheme.typography.bodyMedium,
        color = MaterialTheme.colorScheme.onSurfaceVariant
    )
    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.spacedBy(6.dp),
        verticalAlignment = Alignment.CenterVertically
    ) {
        SegmentNumberField(
            value = year,
            onValueChange = { onYearChange(filterDigits(it, 4)) },
            keyboardOptions = keyboardOptions,
            width = SegmentYearWidth,
            placeholder = "YYYY"
        )
        IsoSeparator("-")
        SegmentNumberField(
            value = month,
            onValueChange = { onMonthChange(filterDigits(it, 2)) },
            keyboardOptions = keyboardOptions,
            width = SegmentShortWidth,
            placeholder = "MM"
        )
    }
}

@Composable
private fun SegmentedYearWeekInput(
    title: String,
    year: String,
    week: String,
    keyboardOptions: KeyboardOptions,
    onYearChange: (String) -> Unit,
    onWeekChange: (String) -> Unit
) {
    Text(
        text = "$title (YYYY-Www)",
        style = MaterialTheme.typography.bodyMedium,
        color = MaterialTheme.colorScheme.onSurfaceVariant
    )
    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.spacedBy(6.dp),
        verticalAlignment = Alignment.CenterVertically
    ) {
        SegmentNumberField(
            value = year,
            onValueChange = { onYearChange(filterDigits(it, 4)) },
            keyboardOptions = keyboardOptions,
            width = SegmentYearWidth,
            placeholder = "YYYY"
        )
        IsoSeparator("-W")
        SegmentNumberField(
            value = week,
            onValueChange = { onWeekChange(filterDigits(it, 2)) },
            keyboardOptions = keyboardOptions,
            width = SegmentShortWidth,
            placeholder = "WW"
        )
    }
}

@Composable
private fun SegmentNumberField(
    value: String,
    onValueChange: (String) -> Unit,
    keyboardOptions: KeyboardOptions,
    width: Dp,
    placeholder: String
) {
    OutlinedTextField(
        value = value,
        onValueChange = onValueChange,
        placeholder = {
            Text(
                text = placeholder,
                modifier = Modifier.fillMaxWidth(),
                textAlign = TextAlign.Center,
                style = MaterialTheme.typography.bodySmall
            )
        },
        singleLine = true,
        keyboardOptions = keyboardOptions,
        textStyle = MaterialTheme.typography.titleMedium.copy(textAlign = TextAlign.Center),
        modifier = Modifier
            .width(width)
            .height(SegmentFieldHeight)
    )
}

@Composable
private fun IsoSeparator(value: String) {
    Text(
        text = value,
        style = MaterialTheme.typography.titleMedium,
        color = MaterialTheme.colorScheme.onSurfaceVariant
    )
}

private fun filterDigits(value: String, maxLength: Int): String =
    value.filter { it.isDigit() }.take(maxLength)

private fun splitDateDigits(value: String): Triple<String, String, String> {
    val digits = filterDigits(value, 8)
    return Triple(
        digits.take(4),
        digits.drop(4).take(2),
        digits.drop(6).take(2)
    )
}

private fun mergeDateDigits(year: String, month: String, day: String): String =
    filterDigits(year, 4) + filterDigits(month, 2) + filterDigits(day, 2)

private fun splitYearMonthDigits(value: String): Pair<String, String> {
    val digits = filterDigits(value, 6)
    return Pair(digits.take(4), digits.drop(4).take(2))
}

private fun mergeYearMonthDigits(year: String, month: String): String =
    filterDigits(year, 4) + filterDigits(month, 2)

private fun splitYearWeekDigits(value: String): Pair<String, String> {
    val digits = filterDigits(value, 6)
    return Pair(digits.take(4), digits.drop(4).take(2))
}

private fun mergeYearWeekDigits(year: String, week: String): String =
    filterDigits(year, 4) + filterDigits(week, 2)

private fun normalizeSignedIntInput(value: String, maxDigits: Int): String {
    if (value.isEmpty()) {
        return ""
    }
    val trimmed = value.trim()
    if (trimmed == "-") {
        return "-"
    }
    val isNegative = trimmed.startsWith("-")
    val digits = trimmed.filter { it.isDigit() }.take(maxDigits)
    return if (isNegative) "-$digits" else digits
}
