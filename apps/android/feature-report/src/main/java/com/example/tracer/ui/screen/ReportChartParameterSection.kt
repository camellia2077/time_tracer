package com.example.tracer

import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.fillMaxWidth
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
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.input.KeyboardType
import com.example.tracer.feature.report.R

@Composable
internal fun ReportChartParameterSection(
    rootOptions: List<String>,
    chartSelectedRoot: String,
    chartDateInputMode: ChartDateInputMode,
    chartLookbackDays: String,
    chartRangeStartDate: String,
    chartRangeEndDate: String,
    chartLoading: Boolean,
    chartLastTrace: ChartQueryTrace?,
    onChartRootChange: (String) -> Unit,
    onChartDateInputModeChange: (ChartDateInputMode) -> Unit,
    onChartLookbackDaysChange: (String) -> Unit,
    onChartRangeStartDateChange: (String) -> Unit,
    onChartRangeEndDateChange: (String) -> Unit,
    onLoadChart: () -> Unit
) {
    var rootMenuExpanded by remember { mutableStateOf(false) }

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

    if (chartLastTrace != null) {
        Text(
            text = "op=${chartLastTrace.operationId} · " +
                "cache=${chartLastTrace.cacheHit} · " +
                "ms=${chartLastTrace.durationMs} · points=${chartLastTrace.pointCount}",
            style = MaterialTheme.typography.bodySmall,
            color = MaterialTheme.colorScheme.onSurfaceVariant,
            modifier = Modifier.fillMaxWidth()
        )
    }
}

private fun ChartDateInputMode.labelRes(): Int =
    when (this) {
        ChartDateInputMode.LOOKBACK -> R.string.report_chart_date_mode_lookback
        ChartDateInputMode.RANGE -> R.string.report_chart_date_mode_range
    }
