package com.example.tracer

import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ChevronRight
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Icon
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import com.example.tracer.feature.insights.R
import com.example.tracer.ui.components.TracerOutlinedTextFieldDefaults

@Composable
internal fun InsightsChartParameterSection(
    chartSemanticMode: InsightsChartSemanticMode,
    rootTree: List<TreeNode>,
    trendChartSelectedRoot: String,
    onOpenRootPicker: () -> Unit
) {
    if (chartSemanticMode == InsightsChartSemanticMode.TREND) {
        val scopeLabel = if (trendChartSelectedRoot.isBlank()) {
            stringResource(R.string.insights_chart_root_all)
        } else {
            formatInsightsChartScopeLabel(trendChartSelectedRoot, rootTree)
        }
        // OutlinedTextField consumes pointer input internally, so putting
        // clickable on its modifier does not reliably receive taps. Keep the
        // field as the visual surface and place a full-size click target above
        // it so the root picker always opens from the whole field.
        Box(modifier = Modifier.fillMaxWidth()) {
            OutlinedTextField(
                value = scopeLabel,
                onValueChange = {},
                readOnly = true,
                label = { Text(stringResource(R.string.insights_label_chart_root)) },
                trailingIcon = {
                    Icon(imageVector = Icons.Default.ChevronRight, contentDescription = null)
                },
                shape = TracerOutlinedTextFieldDefaults.shape,
                modifier = Modifier.fillMaxWidth()
            )
            Box(
                modifier = Modifier
                    .matchParentSize()
                    .clickable(onClick = onOpenRootPicker)
            )
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
