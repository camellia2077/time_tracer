package com.example.tracer

import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ArrowDropDown
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
import com.example.tracer.feature.insights.R
import com.example.tracer.ui.components.TracerOutlinedTextFieldDefaults

@Composable
internal fun InsightsChartParameterSection(
    chartSemanticMode: InsightsChartSemanticMode,
    rootOptions: List<String>,
    trendChartSelectedRoot: String,
    onChartRootChange: (String) -> Unit
) {
    var rootMenuExpanded by remember { mutableStateOf(false) }

    if (chartSemanticMode == InsightsChartSemanticMode.TREND) {
        Box(modifier = Modifier.fillMaxWidth()) {
            OutlinedTextField(
                value = if (trendChartSelectedRoot.isBlank()) {
                    stringResource(R.string.insights_chart_root_all)
                } else {
                    trendChartSelectedRoot
                },
                onValueChange = {},
                readOnly = true,
                label = { Text(stringResource(R.string.insights_label_chart_root)) },
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
                        stringResource(R.string.insights_chart_root_all)
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
    } else {
        Text(
            text = stringResource(R.string.insights_chart_composition_scope_hint),
            style = MaterialTheme.typography.bodySmall,
            color = MaterialTheme.colorScheme.onSurfaceVariant,
            modifier = Modifier.fillMaxWidth()
        )
    }
}
