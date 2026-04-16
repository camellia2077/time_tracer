package com.example.tracer

import androidx.compose.animation.AnimatedVisibility
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.ExperimentalLayoutApi
import androidx.compose.foundation.layout.FlowRow
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.selection.selectable
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Check
import androidx.compose.material3.ElevatedCard
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.SegmentedButton
import androidx.compose.material3.SegmentedButtonDefaults
import androidx.compose.material3.SingleChoiceSegmentedButtonRow
import androidx.compose.material3.Switch
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.luminance
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.semantics.Role
import androidx.compose.ui.semantics.contentDescription
import androidx.compose.ui.semantics.semantics
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import com.example.tracer.data.AppLanguage
import com.example.tracer.data.DarkThemeStyle
import com.example.tracer.data.ThemeColor
import com.example.tracer.data.ThemeConfig
import com.example.tracer.data.ThemeMode
import com.example.tracer.ui.components.TracerSegmentedButtonDefaults

@OptIn(ExperimentalMaterial3Api::class)
@Composable
internal fun AppearanceSettingsCard(
    themeConfig: ThemeConfig,
    onSetThemeColor: (ThemeColor) -> Unit,
    onSetThemeMode: (ThemeMode) -> Unit,
    onSetUseDynamicColor: (Boolean) -> Unit,
    onSetDarkThemeStyle: (DarkThemeStyle) -> Unit,
    reportPiePalettePreset: ReportPiePalettePreset,
    onReportPiePalettePresetChange: (ReportPiePalettePreset) -> Unit,
    appLanguage: AppLanguage,
    onSetAppLanguage: (AppLanguage) -> Unit
) {
    ElevatedCard(modifier = Modifier.fillMaxWidth()) {
        androidx.compose.foundation.layout.Column(
            modifier = Modifier.padding(16.dp),
            verticalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            Text(
                text = stringResource(R.string.config_title_appearance),
                style = MaterialTheme.typography.titleMedium,
                color = MaterialTheme.colorScheme.primary
            )

            DynamicColorSection(
                useDynamicColor = themeConfig.useDynamicColor,
                onSetUseDynamicColor = onSetUseDynamicColor
            )

            if (!themeConfig.useDynamicColor) {
                ThemeColorSection(
                    selectedThemeColor = themeConfig.themeColor,
                    onSetThemeColor = onSetThemeColor
                )
            }

            HorizontalDivider()

            ThemeModeSection(
                selectedThemeMode = themeConfig.themeMode,
                onSetThemeMode = onSetThemeMode
            )

            HorizontalDivider()

            LanguageSection(
                appLanguage = appLanguage,
                onSetAppLanguage = onSetAppLanguage
            )

            HorizontalDivider()

            ReportChartStyleSection(
                reportPiePalettePreset = reportPiePalettePreset,
                onReportPiePalettePresetChange = onReportPiePalettePresetChange
            )

            val isSystemDark = isSystemInDarkTheme()
            val isDarkActive = themeConfig.themeMode == ThemeMode.Dark ||
                (themeConfig.themeMode == ThemeMode.System && isSystemDark)

            if (isDarkActive) {
                HorizontalDivider()
                DarkThemeStyleSection(
                    selectedDarkThemeStyle = themeConfig.darkThemeStyle,
                    onSetDarkThemeStyle = onSetDarkThemeStyle
                )
            }
        }
    }
}

@Composable
private fun DynamicColorSection(
    useDynamicColor: Boolean,
    onSetUseDynamicColor: (Boolean) -> Unit
) {
    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.SpaceBetween,
        verticalAlignment = Alignment.CenterVertically
    ) {
        Text(
            text = stringResource(R.string.config_label_use_wallpaper_colors),
            style = MaterialTheme.typography.bodyMedium,
            color = MaterialTheme.colorScheme.onSurfaceVariant
        )
        Switch(
            checked = useDynamicColor,
            onCheckedChange = onSetUseDynamicColor
        )
    }
}

@Composable
private fun ThemeColorSection(
    selectedThemeColor: ThemeColor,
    onSetThemeColor: (ThemeColor) -> Unit
) {
    var isThemeColorExpanded by rememberSaveable { mutableStateOf(false) }

    ExpandableSettingsButton(
        text = if (isThemeColorExpanded) {
            stringResource(R.string.config_theme_color_current, selectedThemeColor.name)
        } else {
            stringResource(R.string.config_theme_color_tap_expand)
        },
        expanded = isThemeColorExpanded,
        onClick = { isThemeColorExpanded = !isThemeColorExpanded }
    )

    AnimatedVisibility(visible = isThemeColorExpanded) {
        @OptIn(ExperimentalLayoutApi::class)
        FlowRow(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.spacedBy(8.dp),
            verticalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            themeColorDisplayOrder().forEach { color ->
                val isSelected = selectedThemeColor == color
                val colorContentDescription = stringResource(
                    R.string.config_theme_color_content_desc,
                    color.name
                )
                val colorValue = resolveThemeColorValue(color)

                Box(
                    modifier = Modifier
                        .size(48.dp)
                        .semantics { contentDescription = colorContentDescription }
                        .selectable(
                            selected = isSelected,
                            onClick = { onSetThemeColor(color) },
                            role = Role.RadioButton
                        ),
                    contentAlignment = Alignment.Center
                ) {
                    Box(
                        modifier = Modifier
                            .size(40.dp)
                            .background(colorValue, CircleShape)
                            .border(
                                width = 1.dp,
                                color = MaterialTheme.colorScheme.outlineVariant,
                                shape = CircleShape
                            ),
                        contentAlignment = Alignment.Center
                    ) {
                        if (isSelected) {
                            Icon(
                                imageVector = Icons.Filled.Check,
                                contentDescription = null,
                                tint = if (colorValue.luminance() > 0.6f) {
                                    Color.Black
                                } else {
                                    Color.White
                                },
                                modifier = Modifier.size(16.dp)
                            )
                        }
                    }

                    if (isSelected) {
                        Box(
                            modifier = Modifier
                                .size(48.dp)
                                .border(
                                    width = 2.dp,
                                    color = MaterialTheme.colorScheme.primary,
                                    shape = CircleShape
                                )
                        )
                    }
                }
            }
        }
    }
}

@Composable
private fun ThemeModeSection(
    selectedThemeMode: ThemeMode,
    onSetThemeMode: (ThemeMode) -> Unit
) {
    Text(
        text = stringResource(R.string.config_title_theme_mode),
        style = MaterialTheme.typography.bodyMedium,
        color = MaterialTheme.colorScheme.onSurfaceVariant
    )

    val themeModes = ThemeMode.entries
    SingleChoiceSegmentedButtonRow(modifier = Modifier.fillMaxWidth()) {
        themeModes.forEachIndexed { index, mode ->
            val selected = selectedThemeMode == mode
            SegmentedButton(
                shape = SegmentedButtonDefaults.itemShape(index = index, count = themeModes.size),
                onClick = { onSetThemeMode(mode) },
                selected = selected,
                modifier = Modifier.weight(1f),
                colors = TracerSegmentedButtonDefaults.colors(),
                label = {
                    Text(
                        text = when (mode) {
                            ThemeMode.System -> stringResource(R.string.config_theme_mode_system)
                            ThemeMode.Light -> stringResource(R.string.config_theme_mode_light)
                            ThemeMode.Dark -> stringResource(R.string.config_theme_mode_dark)
                        },
                        fontWeight = segmentedButtonLabelWeight(selected)
                    )
                }
            )
        }
    }
}

@Composable
private fun LanguageSection(
    appLanguage: AppLanguage,
    onSetAppLanguage: (AppLanguage) -> Unit
) {
    Text(
        text = stringResource(R.string.config_title_language),
        style = MaterialTheme.typography.bodyMedium,
        color = MaterialTheme.colorScheme.onSurfaceVariant
    )

    val appLanguages = listOf(
        AppLanguage.Chinese to stringResource(R.string.config_language_chinese),
        AppLanguage.English to stringResource(R.string.config_language_english),
        AppLanguage.Japanese to stringResource(R.string.config_language_japanese)
    )
    SingleChoiceSegmentedButtonRow(modifier = Modifier.fillMaxWidth()) {
        appLanguages.forEachIndexed { index, (language, label) ->
            val selected = appLanguage == language
            SegmentedButton(
                shape = SegmentedButtonDefaults.itemShape(index = index, count = appLanguages.size),
                onClick = { onSetAppLanguage(language) },
                selected = selected,
                modifier = Modifier.weight(1f),
                colors = TracerSegmentedButtonDefaults.colors(),
                label = {
                    Text(
                        text = label,
                        fontWeight = segmentedButtonLabelWeight(selected)
                    )
                }
            )
        }
    }
}

@Composable
private fun DarkThemeStyleSection(
    selectedDarkThemeStyle: DarkThemeStyle,
    onSetDarkThemeStyle: (DarkThemeStyle) -> Unit
) {
    Text(
        text = stringResource(R.string.config_title_dark_theme_style),
        style = MaterialTheme.typography.bodyMedium,
        color = MaterialTheme.colorScheme.onSurfaceVariant
    )

    val darkThemeStyles = DarkThemeStyle.entries
    SingleChoiceSegmentedButtonRow(modifier = Modifier.fillMaxWidth()) {
        darkThemeStyles.forEachIndexed { index, style ->
            SegmentedButton(
                shape = SegmentedButtonDefaults.itemShape(index = index, count = darkThemeStyles.size),
                onClick = { onSetDarkThemeStyle(style) },
                selected = selectedDarkThemeStyle == style,
                modifier = Modifier.weight(1f),
                label = {
                    Text(
                        text = when (style) {
                            DarkThemeStyle.Tinted -> stringResource(R.string.config_dark_theme_style_tinted)
                            DarkThemeStyle.Neutral -> stringResource(R.string.config_dark_theme_style_grey)
                            DarkThemeStyle.Black -> stringResource(R.string.config_dark_theme_style_black)
                        },
                        style = MaterialTheme.typography.labelSmall
                    )
                }
            )
        }
    }
}

private fun segmentedButtonLabelWeight(selected: Boolean): FontWeight =
    if (selected) {
        TracerSegmentedButtonDefaults.activeLabelFontWeight
    } else {
        TracerSegmentedButtonDefaults.inactiveLabelFontWeight
    }

private fun themeColorDisplayOrder(): List<ThemeColor> {
    val rainbowThemeColors = listOf(
        ThemeColor.Crimson,
        ThemeColor.Burgundy,
        ThemeColor.Rose,
        ThemeColor.Sakura,
        ThemeColor.Peach,
        ThemeColor.Orange,
        ThemeColor.Amber,
        ThemeColor.Gold,
        ThemeColor.Lime,
        ThemeColor.Mint,
        ThemeColor.Emerald,
        ThemeColor.Teal,
        ThemeColor.Turquoise,
        ThemeColor.Cyan,
        ThemeColor.Sky,
        ThemeColor.Cobalt,
        ThemeColor.Periwinkle,
        ThemeColor.Navy,
        ThemeColor.Slate,
        ThemeColor.Lavender,
        ThemeColor.Violet,
        ThemeColor.Magenta,
        ThemeColor.Pink
    )
    val neutralThemeColors = listOf(
        ThemeColor.Cocoa,
        ThemeColor.Graphite
    )
    return rainbowThemeColors +
        neutralThemeColors +
        ThemeColor.entries.filter { color ->
            color !in rainbowThemeColors && color !in neutralThemeColors
        }
}

private fun resolveThemeColorValue(color: ThemeColor): Color =
    when (color) {
        ThemeColor.Rose -> com.example.tracer.ui.theme.Rose500
        ThemeColor.Orange -> com.example.tracer.ui.theme.Orange500
        ThemeColor.Peach -> com.example.tracer.ui.theme.Peach500
        ThemeColor.Amber -> com.example.tracer.ui.theme.Amber500
        ThemeColor.Gold -> com.example.tracer.ui.theme.Gold500
        ThemeColor.Mint -> com.example.tracer.ui.theme.Mint500
        ThemeColor.Emerald -> com.example.tracer.ui.theme.Emerald500
        ThemeColor.Teal -> com.example.tracer.ui.theme.Teal500
        ThemeColor.Turquoise -> com.example.tracer.ui.theme.Turquoise500
        ThemeColor.Cyan -> com.example.tracer.ui.theme.Cyan500
        ThemeColor.Sky -> com.example.tracer.ui.theme.Sky500
        ThemeColor.Periwinkle -> com.example.tracer.ui.theme.Periwinkle500
        ThemeColor.Lavender -> com.example.tracer.ui.theme.Lavender500
        ThemeColor.Violet -> com.example.tracer.ui.theme.Violet500
        ThemeColor.Pink -> com.example.tracer.ui.theme.Pink500
        ThemeColor.Sakura -> com.example.tracer.ui.theme.Sakura500
        ThemeColor.Magenta -> com.example.tracer.ui.theme.Magenta500
        ThemeColor.Cobalt -> com.example.tracer.ui.theme.Cobalt500
        ThemeColor.Navy -> com.example.tracer.ui.theme.Navy500
        ThemeColor.Crimson -> com.example.tracer.ui.theme.Crimson500
        ThemeColor.Burgundy -> com.example.tracer.ui.theme.Burgundy500
        ThemeColor.Lime -> com.example.tracer.ui.theme.Lime500
        ThemeColor.Cocoa -> com.example.tracer.ui.theme.Cocoa500
        ThemeColor.Graphite -> com.example.tracer.ui.theme.Graphite500
        ThemeColor.Slate -> com.example.tracer.ui.theme.Indigo500
    }
