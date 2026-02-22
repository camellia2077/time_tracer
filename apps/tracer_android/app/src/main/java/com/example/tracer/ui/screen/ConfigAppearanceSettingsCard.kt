package com.example.tracer

import androidx.compose.animation.AnimatedVisibility
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.ExperimentalLayoutApi
import androidx.compose.foundation.layout.FlowRow
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.selection.selectable
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Check
import androidx.compose.material.icons.filled.ExpandLess
import androidx.compose.material.icons.filled.ExpandMore
import androidx.compose.material3.ElevatedCard
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.SegmentedButton
import androidx.compose.material3.SegmentedButtonDefaults
import androidx.compose.material3.SingleChoiceSegmentedButtonRow
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
import androidx.compose.ui.unit.dp

@OptIn(ExperimentalMaterial3Api::class)
@Composable
internal fun AppearanceSettingsCard(
    themeConfig: com.example.tracer.data.ThemeConfig,
    onSetThemeColor: (com.example.tracer.data.ThemeColor) -> Unit,
    onSetThemeMode: (com.example.tracer.data.ThemeMode) -> Unit,
    onSetUseDynamicColor: (Boolean) -> Unit,
    onSetDarkThemeStyle: (com.example.tracer.data.DarkThemeStyle) -> Unit,
    appLanguage: com.example.tracer.data.AppLanguage,
    onSetAppLanguage: (com.example.tracer.data.AppLanguage) -> Unit
) {
    ElevatedCard(modifier = Modifier.fillMaxWidth()) {
        Column(
            modifier = Modifier.padding(16.dp),
            verticalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            Text(
                text = stringResource(R.string.config_title_appearance),
                style = MaterialTheme.typography.titleMedium,
                color = MaterialTheme.colorScheme.primary
            )

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
                androidx.compose.material3.Switch(
                    checked = themeConfig.useDynamicColor,
                    onCheckedChange = onSetUseDynamicColor
                )
            }

            if (!themeConfig.useDynamicColor) {
                var isThemeColorExpanded by rememberSaveable { mutableStateOf(false) }

                OutlinedButton(
                    onClick = { isThemeColorExpanded = !isThemeColorExpanded },
                    modifier = Modifier.fillMaxWidth()
                ) {
                    Text(
                        text = if (isThemeColorExpanded) {
                            stringResource(R.string.config_theme_color_current, themeConfig.themeColor.name)
                        } else {
                            stringResource(R.string.config_theme_color_tap_expand)
                        },
                        style = MaterialTheme.typography.bodyMedium
                    )
                    Spacer(modifier = Modifier.weight(1f))
                    Icon(
                        imageVector = if (isThemeColorExpanded) {
                            Icons.Filled.ExpandLess
                        } else {
                            Icons.Filled.ExpandMore
                        },
                        contentDescription = null
                    )
                }

                val rainbowThemeColors = listOf(
                    com.example.tracer.data.ThemeColor.Crimson,
                    com.example.tracer.data.ThemeColor.Burgundy,
                    com.example.tracer.data.ThemeColor.Rose,
                    com.example.tracer.data.ThemeColor.Sakura,
                    com.example.tracer.data.ThemeColor.Peach,
                    com.example.tracer.data.ThemeColor.Orange,
                    com.example.tracer.data.ThemeColor.Amber,
                    com.example.tracer.data.ThemeColor.Gold,
                    com.example.tracer.data.ThemeColor.Lime,
                    com.example.tracer.data.ThemeColor.Mint,
                    com.example.tracer.data.ThemeColor.Emerald,
                    com.example.tracer.data.ThemeColor.Teal,
                    com.example.tracer.data.ThemeColor.Turquoise,
                    com.example.tracer.data.ThemeColor.Cyan,
                    com.example.tracer.data.ThemeColor.Sky,
                    com.example.tracer.data.ThemeColor.Cobalt,
                    com.example.tracer.data.ThemeColor.Periwinkle,
                    com.example.tracer.data.ThemeColor.Navy,
                    com.example.tracer.data.ThemeColor.Slate,
                    com.example.tracer.data.ThemeColor.Lavender,
                    com.example.tracer.data.ThemeColor.Violet,
                    com.example.tracer.data.ThemeColor.Magenta,
                    com.example.tracer.data.ThemeColor.Pink
                )
                val neutralThemeColors = listOf(
                    com.example.tracer.data.ThemeColor.Cocoa,
                    com.example.tracer.data.ThemeColor.Graphite
                )
                val themeColorDisplayOrder = rainbowThemeColors +
                    neutralThemeColors +
                    com.example.tracer.data.ThemeColor.entries.filter { color ->
                        color !in rainbowThemeColors && color !in neutralThemeColors
                    }

                AnimatedVisibility(visible = isThemeColorExpanded) {
                    @OptIn(ExperimentalLayoutApi::class)
                    FlowRow(
                        modifier = Modifier.fillMaxWidth(),
                        horizontalArrangement = Arrangement.spacedBy(8.dp),
                        verticalArrangement = Arrangement.spacedBy(8.dp)
                    ) {
                        themeColorDisplayOrder.forEach { color ->
                            val isSelected = themeConfig.themeColor == color
                            val colorContentDescription = stringResource(
                                R.string.config_theme_color_content_desc,
                                color.name
                            )
                            val colorValue = when (color) {
                                com.example.tracer.data.ThemeColor.Rose -> com.example.tracer.ui.theme.Rose500
                                com.example.tracer.data.ThemeColor.Orange -> com.example.tracer.ui.theme.Orange500
                                com.example.tracer.data.ThemeColor.Peach -> com.example.tracer.ui.theme.Peach500
                                com.example.tracer.data.ThemeColor.Amber -> com.example.tracer.ui.theme.Amber500
                                com.example.tracer.data.ThemeColor.Gold -> com.example.tracer.ui.theme.Gold500
                                com.example.tracer.data.ThemeColor.Mint -> com.example.tracer.ui.theme.Mint500
                                com.example.tracer.data.ThemeColor.Emerald -> com.example.tracer.ui.theme.Emerald500
                                com.example.tracer.data.ThemeColor.Teal -> com.example.tracer.ui.theme.Teal500
                                com.example.tracer.data.ThemeColor.Turquoise -> com.example.tracer.ui.theme.Turquoise500
                                com.example.tracer.data.ThemeColor.Cyan -> com.example.tracer.ui.theme.Cyan500
                                com.example.tracer.data.ThemeColor.Sky -> com.example.tracer.ui.theme.Sky500
                                com.example.tracer.data.ThemeColor.Periwinkle -> com.example.tracer.ui.theme.Periwinkle500
                                com.example.tracer.data.ThemeColor.Lavender -> com.example.tracer.ui.theme.Lavender500
                                com.example.tracer.data.ThemeColor.Violet -> com.example.tracer.ui.theme.Violet500
                                com.example.tracer.data.ThemeColor.Pink -> com.example.tracer.ui.theme.Pink500
                                com.example.tracer.data.ThemeColor.Sakura -> com.example.tracer.ui.theme.Sakura500
                                com.example.tracer.data.ThemeColor.Magenta -> com.example.tracer.ui.theme.Magenta500
                                com.example.tracer.data.ThemeColor.Cobalt -> com.example.tracer.ui.theme.Cobalt500
                                com.example.tracer.data.ThemeColor.Navy -> com.example.tracer.ui.theme.Navy500
                                com.example.tracer.data.ThemeColor.Crimson -> com.example.tracer.ui.theme.Crimson500
                                com.example.tracer.data.ThemeColor.Burgundy -> com.example.tracer.ui.theme.Burgundy500
                                com.example.tracer.data.ThemeColor.Lime -> com.example.tracer.ui.theme.Lime500
                                com.example.tracer.data.ThemeColor.Cocoa -> com.example.tracer.ui.theme.Cocoa500
                                com.example.tracer.data.ThemeColor.Graphite -> com.example.tracer.ui.theme.Graphite500
                                com.example.tracer.data.ThemeColor.Slate -> com.example.tracer.ui.theme.Indigo500
                            }

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

            HorizontalDivider()

            Text(
                text = stringResource(R.string.config_title_theme_mode),
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )

            val themeModes = com.example.tracer.data.ThemeMode.entries
            SingleChoiceSegmentedButtonRow(modifier = Modifier.fillMaxWidth()) {
                themeModes.forEachIndexed { index, mode ->
                    SegmentedButton(
                        shape = SegmentedButtonDefaults.itemShape(index = index, count = themeModes.size),
                        onClick = { onSetThemeMode(mode) },
                        selected = themeConfig.themeMode == mode,
                        modifier = Modifier.weight(1f),
                        label = {
                            Text(
                                text = when (mode) {
                                    com.example.tracer.data.ThemeMode.System -> stringResource(R.string.config_theme_mode_system)
                                    com.example.tracer.data.ThemeMode.Light -> stringResource(R.string.config_theme_mode_light)
                                    com.example.tracer.data.ThemeMode.Dark -> stringResource(R.string.config_theme_mode_dark)
                                }
                            )
                        }
                    )
                }
            }

            HorizontalDivider()

            Text(
                text = stringResource(R.string.config_title_language),
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )

            val appLanguages = listOf(
                com.example.tracer.data.AppLanguage.Chinese to stringResource(R.string.config_language_chinese),
                com.example.tracer.data.AppLanguage.English to stringResource(R.string.config_language_english),
                com.example.tracer.data.AppLanguage.Japanese to stringResource(R.string.config_language_japanese)
            )
            SingleChoiceSegmentedButtonRow(modifier = Modifier.fillMaxWidth()) {
                appLanguages.forEachIndexed { index, (language, label) ->
                    SegmentedButton(
                        shape = SegmentedButtonDefaults.itemShape(index = index, count = appLanguages.size),
                        onClick = { onSetAppLanguage(language) },
                        selected = appLanguage == language,
                        modifier = Modifier.weight(1f),
                        label = { Text(label) }
                    )
                }
            }

            val isSystemDark = isSystemInDarkTheme()
            val isDarkActive = themeConfig.themeMode == com.example.tracer.data.ThemeMode.Dark ||
                (themeConfig.themeMode == com.example.tracer.data.ThemeMode.System && isSystemDark)

            if (isDarkActive) {
                HorizontalDivider()

                Text(
                    text = stringResource(R.string.config_title_dark_theme_style),
                    style = MaterialTheme.typography.bodyMedium,
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )

                val darkThemeStyles = com.example.tracer.data.DarkThemeStyle.entries
                SingleChoiceSegmentedButtonRow(modifier = Modifier.fillMaxWidth()) {
                    darkThemeStyles.forEachIndexed { index, style ->
                        SegmentedButton(
                            shape = SegmentedButtonDefaults.itemShape(index = index, count = darkThemeStyles.size),
                            onClick = { onSetDarkThemeStyle(style) },
                            selected = themeConfig.darkThemeStyle == style,
                            modifier = Modifier.weight(1f),
                            label = {
                                Text(
                                    text = when (style) {
                                        com.example.tracer.data.DarkThemeStyle.Tinted -> stringResource(R.string.config_dark_theme_style_tinted)
                                        com.example.tracer.data.DarkThemeStyle.Neutral -> stringResource(R.string.config_dark_theme_style_grey)
                                        com.example.tracer.data.DarkThemeStyle.Black -> stringResource(R.string.config_dark_theme_style_black)
                                    },
                                    style = MaterialTheme.typography.labelSmall
                                )
                            }
                        )
                    }
                }
            }
        }
    }
}
