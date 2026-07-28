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
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
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

@Composable
internal fun ThemePaletteSection(
    selectedThemePalette: ThemePalette,
    onSetThemePalette: (ThemePalette) -> Unit
) {
    Text(
        text = stringResource(R.string.config_title_theme_palette),
        style = MaterialTheme.typography.bodyMedium,
        color = MaterialTheme.colorScheme.onSurfaceVariant
    )

    val switchablePalettes = ThemePalette.entries.filter { it.supportsLightDarkMode }
    val fixedPalettes = ThemePalette.entries.filterNot { it.supportsLightDarkMode }

    Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
        ThemePaletteRows(
            palettes = switchablePalettes,
            selectedThemePalette = selectedThemePalette,
            onSetThemePalette = onSetThemePalette
        )
        HorizontalDivider(modifier = Modifier.padding(vertical = 4.dp))
        ThemePaletteRows(
            palettes = fixedPalettes,
            selectedThemePalette = selectedThemePalette,
            onSetThemePalette = onSetThemePalette
        )
    }
}

@Composable
private fun ThemePaletteRows(
    palettes: List<ThemePalette>,
    selectedThemePalette: ThemePalette,
    onSetThemePalette: (ThemePalette) -> Unit
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
    modifier: Modifier = Modifier
) {
    val label = when (palette) {
        ThemePalette.Indigo -> stringResource(R.string.config_theme_palette_indigo)
        ThemePalette.GraphiteAmber -> stringResource(R.string.config_theme_palette_graphite)
        ThemePalette.Teal -> stringResource(R.string.config_theme_palette_teal)
        ThemePalette.Orange -> stringResource(R.string.config_theme_palette_orange)
        ThemePalette.Rose -> stringResource(R.string.config_theme_palette_rose)
        ThemePalette.Amber -> stringResource(R.string.config_theme_palette_amber)
        ThemePalette.Parchment -> stringResource(R.string.config_theme_palette_parchment)
        ThemePalette.Snowfield -> stringResource(R.string.config_theme_palette_snowfield)
        ThemePalette.Blueprint -> stringResource(R.string.config_theme_palette_blueprint)
        ThemePalette.Newsprint -> stringResource(R.string.config_theme_palette_newsprint)
        ThemePalette.InkWash -> stringResource(R.string.config_theme_palette_ink_wash)
        ThemePalette.Kraft -> stringResource(R.string.config_theme_palette_kraft)
    }
    val colors = palette.definition().preview
    val shape = RoundedCornerShape(12.dp)

    Surface(
        modifier = modifier
            .clip(shape)
            .clickable(onClick = onClick)
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
