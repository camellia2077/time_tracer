package com.example.tracer

import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.fillMaxWidth
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
import com.example.tracer.feature.report.R
import com.example.tracer.ui.components.TracerOutlinedTextFieldDefaults

@Composable
internal fun ReportChartParameterSection(
    rootOptions: List<String>,
    chartSelectedRoot: String,
    chartLoading: Boolean,
    chartLastTrace: ChartQueryTrace?,
    onChartRootChange: (String) -> Unit,
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
            shape = TracerOutlinedTextFieldDefaults.shape,
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
