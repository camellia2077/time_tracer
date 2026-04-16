package com.example.tracer

import androidx.compose.animation.AnimatedVisibility
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.selection.selectable
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
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
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.testTag
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.semantics.Role
import androidx.compose.ui.unit.dp
import android.graphics.Color as AndroidColor

private const val REPORT_PALETTE_SUMMARY_SWATCHES_TAG = "config_report_palette_summary_swatches"
private const val REPORT_PALETTE_EXPANDED_CONTENT_TAG = "config_report_palette_expanded_content"
private const val REPORT_PALETTE_EXPANDED_BAR_PREVIEW_TAG = "config_report_palette_expanded_bar_preview"
private const val REPORT_PALETTE_PRESET_SWATCH_TAG = "config_report_palette_preset_swatches"

@Composable
internal fun ReportChartStyleSection(
    reportPiePalettePreset: ReportPiePalettePreset,
    onReportPiePalettePresetChange: (ReportPiePalettePreset) -> Unit
) {
    Text(
        text = stringResource(R.string.config_title_report_chart_style),
        style = MaterialTheme.typography.bodyMedium,
        color = MaterialTheme.colorScheme.onSurfaceVariant
    )
    var isPiePaletteExpanded by rememberSaveable { mutableStateOf(false) }
    val piePalettePresets = ReportPiePalettePreset.entries
    var previewSliceIndex by rememberSaveable { mutableStateOf(0) }

    ExpandableSettingsButton(
        text = if (isPiePaletteExpanded) {
            stringResource(
                R.string.config_report_breakdown_palette_current,
                stringResource(reportPiePalettePreset.labelRes())
            )
        } else {
            stringResource(R.string.config_report_breakdown_palette_tap_expand)
        },
        expanded = isPiePaletteExpanded,
        onClick = { isPiePaletteExpanded = !isPiePaletteExpanded },
        previewContent = {
            // The collapsed row stays summary-only so the button never turns into a
            // second full preview container.
            ReportBreakdownPaletteSummaryPreview(
                palettePreset = reportPiePalettePreset,
                modifier = Modifier.fillMaxWidth()
            )
        }
    )

    AnimatedVisibility(visible = isPiePaletteExpanded) {
        Column(
            modifier = Modifier.fillMaxWidth(),
            verticalArrangement = Arrangement.spacedBy(10.dp)
        ) {
            Text(
                text = stringResource(R.string.config_label_report_breakdown_palette),
                style = MaterialTheme.typography.labelSmall,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
            Text(
                text = stringResource(R.string.config_report_breakdown_palette_preview_hint),
                style = MaterialTheme.typography.bodySmall,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
            ReportBreakdownPaletteExpandedPreview(
                palettePreset = reportPiePalettePreset,
                selectedIndex = previewSliceIndex,
                onSelectedIndexChange = { previewSliceIndex = it },
                modifier = Modifier.fillMaxWidth()
            )
            piePalettePresets.forEach { preset ->
                val isSelected = preset == reportPiePalettePreset
                OutlinedButton(
                    onClick = { onReportPiePalettePresetChange(preset) },
                    modifier = Modifier.fillMaxWidth()
                ) {
                    Column(
                        modifier = Modifier.fillMaxWidth(),
                        verticalArrangement = Arrangement.spacedBy(6.dp)
                    ) {
                        Text(
                            text = stringResource(preset.labelRes()),
                            style = MaterialTheme.typography.bodyMedium,
                            color = if (isSelected) {
                                MaterialTheme.colorScheme.primary
                            } else {
                                MaterialTheme.colorScheme.onSurface
                            }
                        )
                        ReportBreakdownPalettePresetPreview(
                            palettePreset = preset,
                            modifier = Modifier.fillMaxWidth()
                        )
                    }
                }
            }
        }
    }
}

@Composable
private fun ReportBreakdownPaletteSummaryPreview(
    palettePreset: ReportPiePalettePreset,
    modifier: Modifier = Modifier
) {
    PiePalettePreviewStrip(
        palettePreset = palettePreset,
        modifier = modifier.testTag(REPORT_PALETTE_SUMMARY_SWATCHES_TAG)
    )
}

@Composable
private fun ReportBreakdownPalettePresetPreview(
    palettePreset: ReportPiePalettePreset,
    modifier: Modifier = Modifier
) {
    PiePalettePreviewStrip(
        palettePreset = palettePreset,
        modifier = modifier.testTag(REPORT_PALETTE_PRESET_SWATCH_TAG)
    )
}

@Composable
private fun PiePalettePreviewStrip(
    palettePreset: ReportPiePalettePreset,
    modifier: Modifier = Modifier
) {
    val outlineColor = MaterialTheme.colorScheme.outlineVariant
    val previewColors = reportPiePaletteHexColors(palettePreset)
        .mapNotNull(::parseConfigPaletteHexColor)
        .ifEmpty { listOf(MaterialTheme.colorScheme.surfaceVariant) }
        .take(5)
        .toMutableList()
        .apply {
            while (size < 5) {
                add(last())
            }
        }

    Row(
        modifier = modifier,
        horizontalArrangement = Arrangement.spacedBy(6.dp),
        verticalAlignment = Alignment.CenterVertically
    ) {
        previewColors.forEach { color ->
            Box(
                modifier = Modifier
                    .size(18.dp)
                    .background(color, CircleShape)
                    .border(1.dp, outlineColor, CircleShape)
            )
        }
        Spacer(modifier = Modifier.width(4.dp))
        Box(
            modifier = Modifier
                .size(18.dp)
                .background(
                    parseConfigPaletteHexColor(reportPiePaletteOthersHexColor())
                        ?: MaterialTheme.colorScheme.surfaceVariant,
                    CircleShape
                )
                .border(1.dp, outlineColor, CircleShape)
        )
    }
}

@Composable
private fun ReportBreakdownPaletteExpandedPreview(
    palettePreset: ReportPiePalettePreset,
    selectedIndex: Int = 0,
    onSelectedIndexChange: (Int) -> Unit = {},
    modifier: Modifier = Modifier
) {
    val previewSlices = rememberConfigPiePreviewSlices()
    Column(
        modifier = modifier.testTag(REPORT_PALETTE_EXPANDED_CONTENT_TAG),
        verticalArrangement = Arrangement.spacedBy(10.dp)
    ) {
        Text(
            text = stringResource(R.string.config_report_breakdown_palette_pie_preview),
            style = MaterialTheme.typography.labelSmall,
            color = MaterialTheme.colorScheme.onSurfaceVariant
        )
        ReportPieChart(
            slices = previewSlices,
            palettePreset = palettePreset,
            selectedIndex = selectedIndex.coerceIn(0, previewSlices.lastIndex),
            onSliceSelected = onSelectedIndexChange,
            modifier = Modifier
                .fillMaxWidth()
                .height(220.dp)
        )
        Text(
            text = stringResource(R.string.config_report_breakdown_palette_bar_preview),
            style = MaterialTheme.typography.labelSmall,
            color = MaterialTheme.colorScheme.onSurfaceVariant
        )
        ConfigBreakdownBarPreview(
            slices = previewSlices.take(4),
            palettePreset = palettePreset,
            selectedIndex = selectedIndex,
            onSelectedIndexChange = onSelectedIndexChange,
            modifier = Modifier.testTag(REPORT_PALETTE_EXPANDED_BAR_PREVIEW_TAG)
        )
    }
}

@Composable
private fun ConfigBreakdownBarPreview(
    slices: List<ReportCompositionSlice>,
    palettePreset: ReportPiePalettePreset,
    selectedIndex: Int,
    onSelectedIndexChange: (Int) -> Unit,
    modifier: Modifier = Modifier
) {
    val outlineColor = MaterialTheme.colorScheme.outlineVariant
    val trackColor = MaterialTheme.colorScheme.surfaceContainerHighest
    val maxDurationSeconds = remember(slices) {
        slices.maxOfOrNull { it.durationSeconds.coerceAtLeast(0L) } ?: 0L
    }
    Column(
        modifier = modifier,
        verticalArrangement = Arrangement.spacedBy(8.dp)
    ) {
        slices.forEachIndexed { index, slice ->
            val isSelected = index == selectedIndex
            val fillFraction = if (maxDurationSeconds > 0L) {
                slice.durationSeconds.coerceAtLeast(0L).toFloat() / maxDurationSeconds.toFloat()
            } else {
                0f
            }
            val barColor = resolveConfigBreakdownColor(
                slice = slice,
                palettePreset = palettePreset
            )
            Column(
                modifier = Modifier
                    .fillMaxWidth()
                    .clip(MaterialTheme.shapes.medium)
                    .background(
                        if (isSelected) {
                            MaterialTheme.colorScheme.primaryContainer.copy(alpha = 0.24f)
                        } else {
                            MaterialTheme.colorScheme.surfaceVariant.copy(alpha = 0.14f)
                        }
                    )
                    .border(
                        width = if (isSelected) 1.5.dp else 1.dp,
                        color = if (isSelected) {
                            MaterialTheme.colorScheme.primary
                        } else {
                            outlineColor
                        },
                        shape = MaterialTheme.shapes.medium
                    )
                    .selectable(
                        selected = isSelected,
                        onClick = { onSelectedIndexChange(index) },
                        role = Role.RadioButton
                    )
                    .padding(horizontal = 12.dp, vertical = 10.dp),
                verticalArrangement = Arrangement.spacedBy(6.dp)
            ) {
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.SpaceBetween,
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Text(
                        text = slice.root,
                        style = MaterialTheme.typography.bodySmall,
                        modifier = Modifier.width(96.dp)
                    )
                    Text(
                        text = "${slice.percent.toInt()}%",
                        style = MaterialTheme.typography.labelSmall,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                }
                Box(
                    modifier = Modifier
                        .fillMaxWidth()
                        .height(10.dp)
                        .clip(RoundedCornerShape(999.dp))
                        .background(trackColor)
                ) {
                    Box(
                        modifier = Modifier
                            .fillMaxWidth(fillFraction.coerceIn(0f, 1f))
                            .height(10.dp)
                            .clip(RoundedCornerShape(999.dp))
                            .background(barColor)
                    )
                }
            }
        }
    }
}

@Composable
private fun rememberConfigPiePreviewSlices(): List<ReportCompositionSlice> = remember {
    // Keep config preview data static so palette changes are easy to compare even when no
    // report composition query has been loaded yet.
    listOf(
        ReportCompositionSlice(root = "study", durationSeconds = 12_600L, percent = 35f),
        ReportCompositionSlice(root = "sleep", durationSeconds = 9_000L, percent = 25f),
        ReportCompositionSlice(root = "exercise", durationSeconds = 5_400L, percent = 15f),
        ReportCompositionSlice(root = "meal", durationSeconds = 3_600L, percent = 10f),
        ReportCompositionSlice(root = "reading", durationSeconds = 2_880L, percent = 8f),
        ReportCompositionSlice(root = "Others", durationSeconds = 2_520L, percent = 7f)
    )
}

private fun ReportPiePalettePreset.labelRes(): Int =
    when (this) {
        ReportPiePalettePreset.SOFT -> R.string.config_report_breakdown_palette_soft
        ReportPiePalettePreset.EDITORIAL -> R.string.config_report_breakdown_palette_editorial
        ReportPiePalettePreset.VIVID -> R.string.config_report_breakdown_palette_vivid
        ReportPiePalettePreset.MONO_ACCENT -> R.string.config_report_breakdown_palette_mono_accent
    }

private fun parseConfigPaletteHexColor(raw: String): Color? =
    runCatching { Color(AndroidColor.parseColor(raw.trim())) }.getOrNull()

private fun resolveConfigBreakdownColor(
    slice: ReportCompositionSlice,
    palettePreset: ReportPiePalettePreset
): Color {
    if (slice.root == "Others") {
        return parseConfigPaletteHexColor(reportPiePaletteOthersHexColor())
            ?: Color(0xFF94A3B8)
    }
    val palette = reportPiePaletteHexColors(palettePreset)
        .mapNotNull(::parseConfigPaletteHexColor)
        .ifEmpty { listOf(Color(0xFF4F46E5)) }
    return palette[slice.root.hashCode().mod(palette.size)]
}
