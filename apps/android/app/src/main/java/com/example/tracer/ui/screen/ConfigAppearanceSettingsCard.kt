package com.example.tracer

import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.ElevatedCard
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import com.example.tracer.data.DarkSurfaceStyle
import com.example.tracer.data.ThemeConfig
import com.example.tracer.data.ThemeMode
import com.example.tracer.data.ThemePalette
import com.example.tracer.ui.viewmodel.ThemeEvent

@Composable
internal fun AppearanceSettingsCard(
    themeConfig: ThemeConfig,
    onThemeEvent: (ThemeEvent) -> Unit,
    themePaletteExpanded: Boolean,
    onThemePaletteExpandedChange: (Boolean) -> Unit,
    expanded: Boolean = true,
    onToggleExpanded: () -> Unit = {}
) {
    val isSystemDark = isSystemInDarkTheme()
    val isDarkActive = themeConfig.palette.supportsLightDarkMode && (
        themeConfig.themeMode == ThemeMode.Dark ||
            (themeConfig.themeMode == ThemeMode.System && isSystemDark)
        )

    ElevatedCard(modifier = Modifier.fillMaxWidth()) {
        Column(
            modifier = Modifier.padding(16.dp),
            verticalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            ConfigCardHeader(
                title = stringResource(R.string.config_title_appearance),
                expanded = expanded,
                onToggleExpanded = onToggleExpanded
            )
            if (expanded) {
                HorizontalDivider()
                ThemeModeSection(
                    selectedThemeMode = themeConfig.themeMode,
                    supportsLightDarkMode = themeConfig.palette.supportsLightDarkMode,
                    onSetThemeMode = { onThemeEvent(ThemeEvent.SetMode(it)) }
                )

                HorizontalDivider()
                ThemePaletteSection(
                    selectedThemePalette = themeConfig.palette,
                    onSetThemePalette = { onThemeEvent(ThemeEvent.SetPalette(it)) },
                    isDarkMode = isDarkActive,
                    expanded = themePaletteExpanded,
                    onToggleExpanded = {
                        onThemePaletteExpandedChange(!themePaletteExpanded)
                    }
                )

                if (isDarkActive) {
                    HorizontalDivider()
                    DarkSurfaceStyleSection(themeConfig.darkSurfaceStyle) {
                        onThemeEvent(ThemeEvent.SetDarkSurfaceStyle(it))
                    }
                } else {
                    if (themeConfig.palette.supportsLightDarkMode) {
                        HorizontalDivider()
                        LightSurfaceStyleSection(
                            selectedLightSurfaceStyle = themeConfig.lightSurfaceStyle,
                            onSetLightSurfaceStyle = {
                                onThemeEvent(ThemeEvent.SetLightSurfaceStyle(it))
                            }
                        )
                    }
                }
            }
        }
    }
}
