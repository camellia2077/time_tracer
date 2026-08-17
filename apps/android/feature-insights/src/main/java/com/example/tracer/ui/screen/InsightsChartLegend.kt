package com.example.tracer

import androidx.compose.foundation.Canvas
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.width
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.PathEffect
import androidx.compose.ui.unit.dp

@Composable
internal fun InsightsChartComparisonLegend(
    comparisonLabel: String,
    currentColor: Color = MaterialTheme.colorScheme.primary,
    comparisonColor: Color = currentColor.copy(alpha = 0.48f)
) {
    Row(
        horizontalArrangement = Arrangement.spacedBy(16.dp),
        verticalAlignment = Alignment.CenterVertically
    ) {
        InsightsChartLegendItem(
            color = currentColor,
            dashed = false,
            label = androidx.compose.ui.res.stringResource(
                com.example.tracer.feature.insights.R.string.insights_chart_legend_current_period
            )
        )
        InsightsChartLegendItem(
            color = comparisonColor,
            dashed = true,
            label = androidx.compose.ui.res.stringResource(
                com.example.tracer.feature.insights.R.string.insights_chart_legend_comparison_period,
                comparisonLabel
            )
        )
    }
}

@Composable
private fun InsightsChartLegendItem(color: Color, dashed: Boolean, label: String) {
    Row(verticalAlignment = Alignment.CenterVertically) {
        Canvas(modifier = Modifier.width(24.dp).height(12.dp)) {
            drawLine(
                color = color,
                start = Offset(0f, size.height / 2f),
                end = Offset(size.width, size.height / 2f),
                strokeWidth = 2.dp.toPx(),
                pathEffect = if (dashed) {
                    PathEffect.dashPathEffect(floatArrayOf(6.dp.toPx(), 4.dp.toPx()), 0f)
                } else {
                    null
                }
            )
        }
        Spacer(modifier = Modifier.width(6.dp))
        Text(text = label, style = MaterialTheme.typography.labelSmall)
    }
}
