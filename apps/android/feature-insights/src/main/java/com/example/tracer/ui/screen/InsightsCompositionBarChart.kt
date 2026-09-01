package com.example.tracer

import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.unit.dp

@Composable
internal fun InsightsCompositionBarChart(
    slices: List<InsightsCompositionSlice>,
    palettePreset: InsightsPiePalettePreset,
    onItemSelected: (Int) -> Unit,
    showAverage: Boolean = true,
    showFrequency: Boolean = false,
    modifier: Modifier = Modifier
) {
    val sliceColors = rememberPieSliceColors(
        slices = slices,
        palettePreset = palettePreset
    )
    Column(
        modifier = modifier,
        verticalArrangement = Arrangement.spacedBy(8.dp)
    ) {
        slices.forEachIndexed { index, slice ->
            val barFraction = (slice.percent / 100f).coerceIn(0f, 1f)
            Column(
                modifier = Modifier
                    .fillMaxWidth()
                    .clip(MaterialTheme.shapes.medium)
                    .background(MaterialTheme.colorScheme.surfaceVariant.copy(alpha = 0.18f))
                    .border(
                        width = 1.dp,
                        color = MaterialTheme.colorScheme.outline,
                        shape = MaterialTheme.shapes.medium
                    )
                    .clickable { onItemSelected(index) }
                    .padding(horizontal = 12.dp, vertical = 10.dp),
                verticalArrangement = Arrangement.spacedBy(8.dp)
            ) {
                CompositionBarLegendRow(
                    slice = slice,
                    showAverage = showAverage,
                    showFrequency = showFrequency
                )
                Box(
                    modifier = Modifier
                        .fillMaxWidth()
                        .height(12.dp)
                        .clip(RoundedCornerShape(999.dp))
                        .background(MaterialTheme.colorScheme.surfaceContainerHighest)
                ) {
                    Box(
                        modifier = Modifier
                            .fillMaxWidth(barFraction)
                            .height(12.dp)
                            .clip(RoundedCornerShape(999.dp))
                            .background(
                                sliceColors.getOrElse(index) {
                                    resolvePieSliceColor(
                                        slice = slice,
                                        palettePreset = palettePreset
                                    )
                                }
                            )
                    )
                }
            }
        }
    }
}
