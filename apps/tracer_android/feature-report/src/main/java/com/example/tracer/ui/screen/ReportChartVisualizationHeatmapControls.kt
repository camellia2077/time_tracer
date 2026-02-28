package com.example.tracer

import android.graphics.Color as AndroidColor
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.shape.RoundedCornerShape
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
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.report.R

@Composable
internal fun ReportChartHeatmapSettings(
    heatmapTomlConfig: ReportHeatmapTomlConfig,
    heatmapStylePreference: ReportHeatmapStylePreference,
    onHeatmapThemePolicyChange: (ReportHeatmapThemePolicy) -> Unit,
    onHeatmapPaletteNameChange: (String) -> Unit,
    heatmapApplyMessage: String
) {
    val isSystemDark = isSystemInDarkTheme()
    Text(
        text = stringResource(R.string.report_chart_heatmap_hint),
        style = MaterialTheme.typography.labelSmall,
        color = MaterialTheme.colorScheme.onSurfaceVariant
    )
    HeatmapThemePolicySelector(
        selectedPolicy = heatmapStylePreference.themePolicy,
        onPolicyChange = { nextPolicy ->
            onHeatmapThemePolicyChange(nextPolicy)
            if (nextPolicy == ReportHeatmapThemePolicy.PALETTE &&
                heatmapStylePreference.paletteName.isBlank()
            ) {
                val candidatePaletteName = when {
                    isSystemDark && heatmapTomlConfig.defaultDarkPalette.isNotBlank() ->
                        heatmapTomlConfig.defaultDarkPalette
                    heatmapTomlConfig.defaultLightPalette.isNotBlank() ->
                        heatmapTomlConfig.defaultLightPalette
                    else -> heatmapTomlConfig.paletteNames().firstOrNull().orEmpty()
                }
                if (candidatePaletteName.isNotBlank()) {
                    onHeatmapPaletteNameChange(candidatePaletteName)
                }
            }
        }
    )
    if (heatmapStylePreference.themePolicy == ReportHeatmapThemePolicy.PALETTE) {
        HeatmapPaletteSelector(
            palettes = heatmapTomlConfig.palettes,
            selectedPaletteName = heatmapStylePreference.paletteName,
            onPaletteSelected = onHeatmapPaletteNameChange
        )
    }
    if (heatmapApplyMessage.isNotBlank()) {
        Text(
            text = heatmapApplyMessage,
            style = MaterialTheme.typography.bodySmall,
            color = MaterialTheme.colorScheme.error
        )
    }
}

@Composable
private fun HeatmapThemePolicySelector(
    selectedPolicy: ReportHeatmapThemePolicy,
    onPolicyChange: (ReportHeatmapThemePolicy) -> Unit
) {
    val options = ReportHeatmapThemePolicy.entries
    Text(
        text = stringResource(R.string.report_chart_heatmap_theme_policy_label),
        style = MaterialTheme.typography.labelSmall,
        color = MaterialTheme.colorScheme.onSurfaceVariant
    )
    SingleChoiceSegmentedButtonRow(modifier = Modifier.fillMaxWidth()) {
        options.forEachIndexed { index, option ->
            SegmentedButton(
                shape = SegmentedButtonDefaults.itemShape(
                    index = index,
                    count = options.size
                ),
                onClick = { onPolicyChange(option) },
                selected = selectedPolicy == option,
                label = {
                    Text(
                        text = stringResource(option.labelRes()),
                        maxLines = 1,
                        softWrap = false,
                        overflow = TextOverflow.Ellipsis
                    )
                }
            )
        }
    }
}

@Composable
private fun HeatmapPaletteSelector(
    palettes: Map<String, List<String>>,
    selectedPaletteName: String,
    onPaletteSelected: (String) -> Unit
) {
    val paletteNames = palettes.keys.sorted()
    if (paletteNames.isEmpty()) {
        return
    }

    var menuExpanded by remember { mutableStateOf(false) }
    val normalizedSelected = if (selectedPaletteName in paletteNames) {
        selectedPaletteName
    } else {
        paletteNames.first()
    }

    Text(
        text = stringResource(R.string.report_chart_heatmap_palette_label),
        style = MaterialTheme.typography.labelSmall,
        color = MaterialTheme.colorScheme.onSurfaceVariant
    )
    Box(modifier = Modifier.fillMaxWidth()) {
        OutlinedTextField(
            value = normalizedSelected,
            onValueChange = {},
            readOnly = true,
            label = { Text(stringResource(R.string.report_chart_heatmap_palette_label)) },
            trailingIcon = {
                IconButton(onClick = { menuExpanded = !menuExpanded }) {
                    Icon(
                        imageVector = Icons.Default.ArrowDropDown,
                        contentDescription = null
                    )
                }
            },
            modifier = Modifier.fillMaxWidth()
        )
        DropdownMenu(
            expanded = menuExpanded,
            onDismissRequest = { menuExpanded = false }
        ) {
            paletteNames.forEach { name ->
                DropdownMenuItem(
                    text = {
                        HeatmapPaletteRow(
                            name = name,
                            paletteColors = palettes[name].orEmpty()
                        )
                    },
                    onClick = {
                        onPaletteSelected(name)
                        menuExpanded = false
                    }
                )
            }
        }
    }
    HeatmapPalettePreviewStrip(
        paletteColors = palettes[normalizedSelected].orEmpty(),
        modifier = Modifier
            .fillMaxWidth()
            .padding(top = 6.dp)
    )
}

@Composable
private fun HeatmapPaletteRow(
    name: String,
    paletteColors: List<String>
) {
    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.SpaceBetween,
        verticalAlignment = Alignment.CenterVertically
    ) {
        Text(
            text = name,
            style = MaterialTheme.typography.bodyMedium.copy(fontWeight = FontWeight.Medium),
            maxLines = 1,
            overflow = TextOverflow.Ellipsis,
            modifier = Modifier.weight(1f)
        )
        HeatmapPalettePreviewStrip(
            paletteColors = paletteColors,
            modifier = Modifier.padding(start = 12.dp)
        )
    }
}

@Composable
private fun HeatmapPalettePreviewStrip(
    paletteColors: List<String>,
    modifier: Modifier = Modifier
) {
    val outlineColor = MaterialTheme.colorScheme.outlineVariant
    val fallbackColor = MaterialTheme.colorScheme.surfaceVariant
    val previewColors = remember(paletteColors, fallbackColor) {
        val resolved = paletteColors
            .mapNotNull(::parsePaletteHexColor)
            .ifEmpty { listOf(fallbackColor) }
            .take(5)
            .toMutableList()
        while (resolved.size < 5) {
            resolved += resolved.last()
        }
        resolved
    }

    Row(
        modifier = modifier,
        horizontalArrangement = Arrangement.spacedBy(4.dp),
        verticalAlignment = Alignment.CenterVertically
    ) {
        previewColors.forEach { color ->
            Box(
                modifier = Modifier
                    .size(14.dp)
                    .clip(RoundedCornerShape(4.dp))
                    .background(color)
                    .border(
                        width = 1.dp,
                        color = outlineColor,
                        shape = RoundedCornerShape(4.dp)
                    )
            )
        }
    }
}

private fun ReportHeatmapThemePolicy.labelRes(): Int =
    when (this) {
        ReportHeatmapThemePolicy.FOLLOW_SYSTEM -> R.string.report_chart_heatmap_theme_policy_system
        ReportHeatmapThemePolicy.PALETTE -> R.string.report_chart_heatmap_theme_policy_palette
    }

private fun parsePaletteHexColor(raw: String): Color? {
    val normalized = raw.trim()
    return runCatching { Color(AndroidColor.parseColor(normalized)) }.getOrNull()
}
