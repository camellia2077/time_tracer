package com.example.tracer

import androidx.annotation.StringRes
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ArrowDropDown
import androidx.compose.material3.Button
import androidx.compose.material3.DropdownMenu
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.SegmentedButton
import androidx.compose.material3.SegmentedButtonDefaults
import androidx.compose.material3.SingleChoiceSegmentedButtonRow
import androidx.compose.material3.Switch
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.report.R

@Composable
fun ReportResultModeSwitcher(
    mode: ReportResultDisplayMode,
    onModeChange: (ReportResultDisplayMode) -> Unit,
    modifier: Modifier = Modifier
) {
    val modes = ReportResultDisplayMode.entries
    SingleChoiceSegmentedButtonRow(modifier = modifier.fillMaxWidth()) {
        modes.forEachIndexed { index, item ->
            SegmentedButton(
                shape = SegmentedButtonDefaults.itemShape(
                    index = index,
                    count = modes.size
                ),
                onClick = { onModeChange(item) },
                selected = mode == item,
                label = { Text(text = stringResource(item.labelRes())) }
            )
        }
    }
}

@Composable
fun ReportChartResultContent(
    chartRoots: List<String>,
    chartSelectedRoot: String,
    chartDateInputMode: ChartDateInputMode,
    chartLookbackDays: String,
    chartRangeStartDate: String,
    chartRangeEndDate: String,
    chartLoading: Boolean,
    chartError: String,
    chartPoints: List<ReportChartPoint>,
    chartAverageDurationSeconds: Long?,
    chartUsesLegacyStatsFallback: Boolean,
    chartShowAverageLine: Boolean,
    onChartRootChange: (String) -> Unit,
    onChartDateInputModeChange: (ChartDateInputMode) -> Unit,
    onChartLookbackDaysChange: (String) -> Unit,
    onChartRangeStartDateChange: (String) -> Unit,
    onChartRangeEndDateChange: (String) -> Unit,
    onChartShowAverageLineChange: (Boolean) -> Unit,
    onLoadChart: () -> Unit,
    modifier: Modifier = Modifier
) {
    val normalizedRoots = remember(chartRoots) { chartRoots.distinct() }
    val rootOptions = remember(normalizedRoots) { listOf("") + normalizedRoots }
    var rootMenuExpanded by remember { mutableStateOf(false) }
    val sortedChartPoints = remember(chartPoints) { chartPoints.sortedBy { it.date } }
    var selectedPointIndex by remember(sortedChartPoints) {
        mutableStateOf(
            if (sortedChartPoints.isEmpty()) {
                -1
            } else {
                sortedChartPoints.lastIndex
            }
        )
    }
    var chartVisualMode by rememberSaveable { mutableStateOf(ReportChartVisualMode.LINE) }

    Column(
        modifier = modifier
            .fillMaxWidth()
            .padding(top = 4.dp),
        verticalArrangement = Arrangement.spacedBy(12.dp)
    ) {
        Text(
            text = stringResource(R.string.report_title_chart_parameters),
            style = MaterialTheme.typography.titleMedium,
            color = MaterialTheme.colorScheme.primary
        )

        Box(modifier = Modifier.fillMaxWidth()) {
            OutlinedTextField(
                value = if (chartSelectedRoot.isBlank()) {
                    stringResource(R.string.report_chart_root_all)
                } else {
                    chartSelectedRoot
                },
                onValueChange = {},
                readOnly = true,
                label = { Text(stringResource(R.string.report_label_chart_root)) },
                trailingIcon = {
                    IconButton(onClick = { rootMenuExpanded = !rootMenuExpanded }) {
                        Icon(
                            imageVector = Icons.Default.ArrowDropDown,
                            contentDescription = null
                        )
                    }
                },
                modifier = Modifier.fillMaxWidth()
            )
            DropdownMenu(
                expanded = rootMenuExpanded,
                onDismissRequest = { rootMenuExpanded = false }
            ) {
                rootOptions.forEach { option ->
                    val label = if (option.isBlank()) {
                        stringResource(R.string.report_chart_root_all)
                    } else {
                        option
                    }
                    DropdownMenuItem(
                        text = { Text(label) },
                        onClick = {
                            onChartRootChange(option)
                            rootMenuExpanded = false
                        }
                    )
                }
            }
        }

        val dateModes = ChartDateInputMode.entries
        Text(
            text = stringResource(R.string.report_label_chart_date_filter_mode),
            style = MaterialTheme.typography.labelSmall,
            color = MaterialTheme.colorScheme.onSurfaceVariant
        )
        SingleChoiceSegmentedButtonRow(modifier = Modifier.fillMaxWidth()) {
            dateModes.forEachIndexed { index, mode ->
                SegmentedButton(
                    shape = SegmentedButtonDefaults.itemShape(
                        index = index,
                        count = dateModes.size
                    ),
                    onClick = { onChartDateInputModeChange(mode) },
                    selected = chartDateInputMode == mode,
                    label = { Text(text = stringResource(mode.labelRes())) }
                )
            }
        }

        if (chartDateInputMode == ChartDateInputMode.LOOKBACK) {
            OutlinedTextField(
                value = chartLookbackDays,
                onValueChange = onChartLookbackDaysChange,
                label = { Text(stringResource(R.string.report_label_chart_lookback_days)) },
                singleLine = true,
                keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Number),
                modifier = Modifier.fillMaxWidth()
            )
        } else {
            val numericKeyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Number)
            val (startYear, startMonth, startDay) = splitDateDigits(chartRangeStartDate)
            SegmentedDateInput(
                title = stringResource(R.string.report_label_chart_range_start),
                year = startYear,
                month = startMonth,
                day = startDay,
                keyboardOptions = numericKeyboardOptions,
                onYearChange = { nextYear ->
                    onChartRangeStartDateChange(
                        mergeDateDigits(nextYear, startMonth, startDay)
                    )
                },
                onMonthChange = { nextMonth ->
                    onChartRangeStartDateChange(
                        mergeDateDigits(startYear, nextMonth, startDay)
                    )
                },
                onDayChange = { nextDay ->
                    onChartRangeStartDateChange(
                        mergeDateDigits(startYear, startMonth, nextDay)
                    )
                }
            )

            val (endYear, endMonth, endDay) = splitDateDigits(chartRangeEndDate)
            SegmentedDateInput(
                title = stringResource(R.string.report_label_chart_range_end),
                year = endYear,
                month = endMonth,
                day = endDay,
                keyboardOptions = numericKeyboardOptions,
                onYearChange = { nextYear ->
                    onChartRangeEndDateChange(mergeDateDigits(nextYear, endMonth, endDay))
                },
                onMonthChange = { nextMonth ->
                    onChartRangeEndDateChange(mergeDateDigits(endYear, nextMonth, endDay))
                },
                onDayChange = { nextDay ->
                    onChartRangeEndDateChange(mergeDateDigits(endYear, endMonth, nextDay))
                }
            )
        }

        Button(
            onClick = onLoadChart,
            enabled = !chartLoading,
            modifier = Modifier.fillMaxWidth()
        ) {
            Text(
                text = if (chartLoading) {
                    stringResource(R.string.report_action_chart_loading)
                } else {
                    stringResource(R.string.report_action_load_chart)
                }
            )
        }

        if (chartError.isNotBlank()) {
            Text(
                text = chartError,
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.error,
                modifier = Modifier.fillMaxWidth()
            )
        } else if (sortedChartPoints.isEmpty()) {
            val showRangeEmpty = chartDateInputMode == ChartDateInputMode.RANGE
            Text(
                text = if (showRangeEmpty) {
                    stringResource(R.string.report_chart_empty_in_range)
                } else {
                    stringResource(R.string.report_chart_empty)
                },
                style = MaterialTheme.typography.bodyMedium,
                modifier = Modifier.fillMaxWidth()
            )
        } else {
            Text(
                text = stringResource(R.string.report_label_chart_visual),
                style = MaterialTheme.typography.labelSmall,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
            SingleChoiceSegmentedButtonRow(modifier = Modifier.fillMaxWidth()) {
                val visualModes = ReportChartVisualMode.entries
                visualModes.forEachIndexed { index, visualMode ->
                    SegmentedButton(
                        shape = SegmentedButtonDefaults.itemShape(
                            index = index,
                            count = visualModes.size
                        ),
                        onClick = { chartVisualMode = visualMode },
                        selected = chartVisualMode == visualMode,
                        label = { Text(text = stringResource(visualMode.labelRes())) }
                    )
                }
            }

            if (chartVisualMode == ReportChartVisualMode.PIE) {
                Text(
                    text = stringResource(R.string.report_chart_pie_hint),
                    style = MaterialTheme.typography.labelSmall,
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )
            } else {
                Text(
                    text = "${stringResource(R.string.report_chart_axis_x_date)} · " +
                        stringResource(R.string.report_chart_axis_y_hours),
                    style = MaterialTheme.typography.labelSmall,
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )
                if (chartVisualMode.supportsAverageLineToggle()) {
                    AverageLineToggleRow(
                        checked = chartShowAverageLine,
                        onCheckedChange = onChartShowAverageLineChange
                    )
                }
            }
            when (chartVisualMode) {
                ReportChartVisualMode.LINE -> {
                    ReportLineChart(
                        points = sortedChartPoints,
                        selectedIndex = selectedPointIndex,
                        averageDurationSeconds = chartAverageDurationSeconds,
                        usesLegacyStatsFallback = chartUsesLegacyStatsFallback,
                        showAverageLine = chartShowAverageLine,
                        onPointSelected = { selectedPointIndex = it },
                        modifier = Modifier
                            .fillMaxWidth()
                            .height(220.dp)
                            .clip(MaterialTheme.shapes.medium)
                    )
                }

                ReportChartVisualMode.BAR -> {
                    ReportBarChart(
                        points = sortedChartPoints,
                        selectedIndex = selectedPointIndex,
                        averageDurationSeconds = chartAverageDurationSeconds,
                        usesLegacyStatsFallback = chartUsesLegacyStatsFallback,
                        showAverageLine = chartShowAverageLine,
                        onPointSelected = { selectedPointIndex = it },
                        modifier = Modifier
                            .fillMaxWidth()
                            .height(220.dp)
                            .clip(MaterialTheme.shapes.medium)
                    )
                }

                ReportChartVisualMode.PIE -> {
                    ReportPieChart(
                        points = sortedChartPoints,
                        selectedIndex = selectedPointIndex,
                        onPointSelected = { selectedPointIndex = it },
                        modifier = Modifier
                            .fillMaxWidth()
                            .height(220.dp)
                            .clip(MaterialTheme.shapes.medium)
                    )
                }
            }

            // Backward-compat fallback for legacy payloads without core stats fields.
            // Planned removal: after one compatibility cycle (target Android v0.3.0).
            val averageDurationSeconds =
                if (chartUsesLegacyStatsFallback) {
                    sortedChartPoints.map { it.durationSeconds.coerceAtLeast(0L) }.average().toLong()
                } else {
                    chartAverageDurationSeconds?.coerceAtLeast(0L) ?: 0L
                }
            Text(
                text = stringResource(
                    R.string.report_chart_average_duration,
                    formatDurationHoursMinutes(averageDurationSeconds)
                ),
                style = MaterialTheme.typography.bodySmall,
                color = MaterialTheme.colorScheme.primary
            )
            if (chartUsesLegacyStatsFallback) {
                Text(
                    text = stringResource(R.string.report_chart_legacy_fallback_notice),
                    style = MaterialTheme.typography.bodySmall,
                    color = MaterialTheme.colorScheme.secondary
                )
            }

            val selectedPoint = sortedChartPoints.getOrNull(selectedPointIndex)
            if (selectedPoint != null) {
                Text(
                    text = stringResource(
                        R.string.report_chart_selected_detail,
                        selectedPoint.date,
                        formatDurationHoursMinutes(selectedPoint.durationSeconds)
                    ),
                    style = MaterialTheme.typography.bodySmall,
                    color = MaterialTheme.colorScheme.primary
                )
            }

            if (chartVisualMode != ReportChartVisualMode.PIE) {
                val start = sortedChartPoints.firstOrNull()?.date?.toMonthDayLabel().orEmpty()
                val middle =
                    sortedChartPoints.getOrNull(sortedChartPoints.size / 2)?.date?.toMonthDayLabel().orEmpty()
                val end = sortedChartPoints.lastOrNull()?.date?.toMonthDayLabel().orEmpty()
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.SpaceBetween,
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Text(text = start, style = MaterialTheme.typography.labelSmall)
                    Text(text = middle, style = MaterialTheme.typography.labelSmall)
                    Text(text = end, style = MaterialTheme.typography.labelSmall)
                }
            }
        }
    }
}

private enum class ReportChartVisualMode {
    LINE,
    BAR,
    PIE
}

@Composable
private fun AverageLineToggleRow(
    checked: Boolean,
    onCheckedChange: (Boolean) -> Unit
) {
    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.SpaceBetween,
        verticalAlignment = Alignment.CenterVertically
    ) {
        Text(
            text = stringResource(R.string.report_chart_toggle_average_line),
            style = MaterialTheme.typography.labelSmall,
            color = MaterialTheme.colorScheme.onSurfaceVariant
        )
        Switch(
            checked = checked,
            onCheckedChange = onCheckedChange
        )
    }
}

private fun ReportChartVisualMode.supportsAverageLineToggle(): Boolean =
    this == ReportChartVisualMode.LINE || this == ReportChartVisualMode.BAR

@StringRes
private fun ChartDateInputMode.labelRes(): Int =
    when (this) {
        ChartDateInputMode.LOOKBACK -> R.string.report_chart_date_mode_lookback
        ChartDateInputMode.RANGE -> R.string.report_chart_date_mode_range
    }

@StringRes
private fun ReportChartVisualMode.labelRes(): Int =
    when (this) {
        ReportChartVisualMode.LINE -> R.string.report_chart_visual_line
        ReportChartVisualMode.BAR -> R.string.report_chart_visual_bar
        ReportChartVisualMode.PIE -> R.string.report_chart_visual_pie
    }
