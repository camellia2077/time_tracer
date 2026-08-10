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
import com.example.tracer.data.DarkThemeStyle
import com.example.tracer.data.ThemeConfig
import com.example.tracer.data.ThemeMode
import com.example.tracer.data.ThemePalette
import com.example.tracer.ui.viewmodel.ThemeEvent

@Composable
internal fun AppearanceSettingsCard(
    themeConfig: ThemeConfig,
    onThemeEvent: (ThemeEvent) -> Unit
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

            HorizontalDivider()
            ThemeModeSection(
                selectedThemeMode = themeConfig.themeMode,
                supportsLightDarkMode = themeConfig.palette.supportsLightDarkMode,
                onSetThemeMode = { onThemeEvent(ThemeEvent.SetMode(it)) }
            )

            HorizontalDivider()
            ThemePaletteSection(themeConfig.palette) { onThemeEvent(ThemeEvent.SetPalette(it)) }

            val isSystemDark = isSystemInDarkTheme()
            val isDarkActive = themeConfig.palette.supportsLightDarkMode && (
                themeConfig.themeMode == ThemeMode.Dark ||
                    (themeConfig.themeMode == ThemeMode.System && isSystemDark)
                )
            if (isDarkActive) {
                HorizontalDivider()
                DarkThemeStyleSection(themeConfig.darkThemeStyle) {
                    onThemeEvent(ThemeEvent.SetDarkStyle(it))
                }
            }
        }
    }
}
