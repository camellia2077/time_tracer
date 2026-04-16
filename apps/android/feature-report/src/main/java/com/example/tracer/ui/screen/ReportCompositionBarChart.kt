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
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.remember
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp

@Composable
internal fun ReportCompositionBarChart(
    slices: List<ReportCompositionSlice>,
    palettePreset: ReportPiePalettePreset,
    selectedIndex: Int,
    onItemSelected: (Int) -> Unit,
    modifier: Modifier = Modifier
) {
    val sliceColors = rememberCompositionSliceColors(
        slices = slices,
        palettePreset = palettePreset
    )
    val maxDurationSeconds = remember(slices) {
        slices.maxOfOrNull { it.durationSeconds.coerceAtLeast(0L) } ?: 0L
    }

    Column(
        modifier = modifier.verticalScroll(rememberScrollState()),
        verticalArrangement = Arrangement.spacedBy(8.dp)
    ) {
        slices.forEachIndexed { index, slice ->
            val isSelected = index == selectedIndex
            val barFraction = if (maxDurationSeconds > 0L) {
                slice.durationSeconds.coerceAtLeast(0L).toFloat() / maxDurationSeconds.toFloat()
            } else {
                0f
            }
            val rowBorderColor = if (isSelected) {
                MaterialTheme.colorScheme.primary
            } else {
                MaterialTheme.colorScheme.outlineVariant
            }
            val rowBackgroundColor = if (isSelected) {
                MaterialTheme.colorScheme.primaryContainer.copy(alpha = 0.28f)
            } else {
                MaterialTheme.colorScheme.surfaceVariant.copy(alpha = 0.18f)
            }

            Column(
                modifier = Modifier
                    .fillMaxWidth()
                    .clip(MaterialTheme.shapes.medium)
                    .background(rowBackgroundColor)
                    .border(
                        width = if (isSelected) 1.5.dp else 1.dp,
                        color = rowBorderColor,
                        shape = MaterialTheme.shapes.medium
                    )
                    .clickable { onItemSelected(index) }
                    .padding(horizontal = 12.dp, vertical = 10.dp),
                verticalArrangement = Arrangement.spacedBy(8.dp)
            ) {
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.SpaceBetween,
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Text(
                        text = slice.root,
                        modifier = Modifier.weight(1f),
                        style = MaterialTheme.typography.bodyMedium,
                        maxLines = 1,
                        overflow = TextOverflow.Ellipsis
                    )
                    Text(
                        text = "${formatDurationHoursMinutes(slice.durationSeconds)}  ${formatPercent(slice.percent)}",
                        style = MaterialTheme.typography.bodySmall,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                }
                Box(
                    modifier = Modifier
                        .fillMaxWidth()
                        .height(12.dp)
                        .clip(RoundedCornerShape(999.dp))
                        .background(MaterialTheme.colorScheme.surfaceContainerHighest)
                ) {
                    Box(
                        modifier = Modifier
                            .fillMaxWidth(barFraction.coerceIn(0f, 1f))
                            .height(12.dp)
                            .clip(RoundedCornerShape(999.dp))
                            .background(
                                sliceColors.getOrElse(index) {
                                    resolveCompositionSliceColor(
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

private fun formatPercent(value: Float): String = String.format("%.1f%%", value)
