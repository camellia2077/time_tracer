package com.example.tracer

import androidx.compose.foundation.BorderStroke
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxHeight
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.platform.testTag
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.semantics.Role
import androidx.compose.ui.semantics.contentDescription
import androidx.compose.ui.semantics.role
import androidx.compose.ui.semantics.selected
import androidx.compose.ui.semantics.semantics
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import com.example.tracer.data.ThemePalette
import com.example.tracer.ui.theme.definition

private const val THEME_PALETTE_SUMMARY_PREVIEW_TAG = "config_theme_palette_summary_preview"
private const val THEME_PALETTE_OPTION_TAG = "config_theme_palette_option"

@Composable
internal fun ThemePaletteSection(
    selectedThemePalette: ThemePalette,
    onSetThemePalette: (ThemePalette) -> Unit,
    isDarkMode: Boolean = false,
    expanded: Boolean,
    onToggleExpanded: () -> Unit
) {
    Text(
        text = stringResource(R.string.config_title_theme_palette),
        style = MaterialTheme.typography.titleMedium,
        color = MaterialTheme.colorScheme.onSurfaceVariant
    )

    ExpandableSettingsButton(
        text = stringResource(selectedThemePalette.labelRes()),
        expanded = expanded,
        onClick = onToggleExpanded,
        previewContent = {
            ThemePaletteSummaryPreview(selectedThemePalette, isDarkMode)
        }
    )

    if (expanded) {
        val switchablePalettes = ThemePalette.entries.filter { it.supportsLightDarkMode }
        val fixedPalettes = ThemePalette.entries.filterNot { it.supportsLightDarkMode }
        var switchableExpanded by rememberSaveable { mutableStateOf(true) }
        var fixedExpanded by rememberSaveable { mutableStateOf(true) }
        Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
            ConfigCardHeader(
                title = stringResource(R.string.config_theme_palette_group_light_dark),
                expanded = switchableExpanded,
                onToggleExpanded = { switchableExpanded = !switchableExpanded },
                titleStyle = MaterialTheme.typography.titleMedium
            )
            if (switchableExpanded) {
                ThemePaletteRows(
                    palettes = switchablePalettes,
                    selectedThemePalette = selectedThemePalette,
                    onSetThemePalette = onSetThemePalette,
                    isDarkMode = isDarkMode
                )
            }
            HorizontalDivider(modifier = Modifier.padding(vertical = 4.dp))
            ConfigCardHeader(
                title = stringResource(R.string.config_theme_palette_group_fixed_appearance),
                expanded = fixedExpanded,
                onToggleExpanded = { fixedExpanded = !fixedExpanded },
                titleStyle = MaterialTheme.typography.titleMedium
            )
            if (fixedExpanded) {
                ThemePaletteRows(
                    palettes = fixedPalettes,
                    selectedThemePalette = selectedThemePalette,
                    onSetThemePalette = onSetThemePalette,
                    isDarkMode = isDarkMode
                )
            }
        }
    }
}

@Composable
private fun ThemePaletteSummaryPreview(palette: ThemePalette, isDarkMode: Boolean) {
    val colors = palette.previewColors(isDarkMode)
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .height(18.dp)
            .clip(RoundedCornerShape(6.dp))
            .testTag(THEME_PALETTE_SUMMARY_PREVIEW_TAG)
    ) {
        Box(
            modifier = Modifier
                .weight(1f)
                .fillMaxHeight()
                .background(colors.primary)
        )
        Box(
            modifier = Modifier
                .weight(1f)
                .fillMaxHeight()
                .background(colors.accent)
        )
        Box(
            modifier = Modifier
                .weight(1f)
                .fillMaxHeight()
                .background(colors.surface)
        )
    }
}

@Composable
private fun ThemePaletteRows(
    palettes: List<ThemePalette>,
    selectedThemePalette: ThemePalette,
    onSetThemePalette: (ThemePalette) -> Unit,
    isDarkMode: Boolean
) {
    palettes.chunked(2).forEach { rowPalettes ->
        // Empty swatches need to fill the row height; otherwise they measure at zero height
        // and their background colors are not drawn.
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            rowPalettes.forEach { palette ->
                ThemePalettePreviewCard(
                    palette = palette,
                    selected = selectedThemePalette == palette,
                    onClick = { onSetThemePalette(palette) },
                    isDarkMode = isDarkMode,
                    modifier = Modifier.weight(1f)
                )
            }
            if (rowPalettes.size < 2) {
                Spacer(modifier = Modifier.weight(1f))
            }
        }
    }
}

@Composable
private fun ThemePalettePreviewCard(
    palette: ThemePalette,
    selected: Boolean,
    onClick: () -> Unit,
    isDarkMode: Boolean,
    modifier: Modifier = Modifier
) {
    val label = stringResource(palette.labelRes())
    val colors = palette.previewColors(isDarkMode)
    val shape = RoundedCornerShape(12.dp)

    Surface(
        modifier = modifier
            .clip(shape)
            .clickable(onClick = onClick)
            .testTag(THEME_PALETTE_OPTION_TAG)
            .semantics {
                this.selected = selected
                this.role = Role.RadioButton
                this.contentDescription = label
            },
        shape = shape,
        color = MaterialTheme.colorScheme.surface,
        border = BorderStroke(
            width = if (selected) 2.dp else 1.dp,
            color = if (selected) MaterialTheme.colorScheme.primary
            else MaterialTheme.colorScheme.outlineVariant
        ),
        tonalElevation = if (selected) 2.dp else 0.dp
    ) {
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .padding(10.dp),
            verticalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .height(32.dp)
                    .clip(RoundedCornerShape(8.dp))
            ) {
                Box(
                    modifier = Modifier
                        .weight(1f)
                        .fillMaxHeight()
                        .background(colors.primary)
                )
                Box(
                    modifier = Modifier
                        .weight(1f)
                        .fillMaxHeight()
                        .background(colors.accent)
                )
                Box(
                    modifier = Modifier
                        .weight(1f)
                        .fillMaxHeight()
                        .background(colors.surface)
                )
            }
            Text(
                text = label,
                style = MaterialTheme.typography.labelLarge,
                fontWeight = if (selected) FontWeight.SemiBold else FontWeight.Normal,
                color = MaterialTheme.colorScheme.onSurface
            )
        }
    }
}

private fun ThemePalette.previewColors(isDarkMode: Boolean): com.example.tracer.ui.theme.ThemePreviewColors {
    val definition = definition()
    return if (isDarkMode && supportsLightDarkMode) {
        com.example.tracer.ui.theme.ThemePreviewColors(
            primary = definition.dark.primary,
            accent = definition.dark.secondary,
            surface = definition.dark.background
        )
    } else {
        definition.preview
    }
}

private fun ThemePalette.labelRes(): Int = when (this) {
    ThemePalette.Indigo -> R.string.config_theme_palette_indigo
    ThemePalette.Purple -> R.string.config_theme_palette_purple
    ThemePalette.Grey -> R.string.config_theme_palette_grey
    ThemePalette.Green -> R.string.config_theme_palette_green
    ThemePalette.Blue -> R.string.config_theme_palette_blue
    ThemePalette.Orange -> R.string.config_theme_palette_orange
    ThemePalette.Yellow -> R.string.config_theme_palette_yellow
    ThemePalette.Rose -> R.string.config_theme_palette_rose
    ThemePalette.Parchment -> R.string.config_theme_palette_parchment
    ThemePalette.Snowfield -> R.string.config_theme_palette_snowfield
    ThemePalette.Blueprint -> R.string.config_theme_palette_blueprint
    ThemePalette.Newsprint -> R.string.config_theme_palette_newsprint
    ThemePalette.InkWash -> R.string.config_theme_palette_ink_wash
    ThemePalette.Kraft -> R.string.config_theme_palette_kraft
    ThemePalette.Linen -> R.string.config_theme_palette_linen
    ThemePalette.Mint -> R.string.config_theme_palette_mint
}
