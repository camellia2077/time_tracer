package com.example.tracer

import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.SegmentedButton
import androidx.compose.material3.SegmentedButtonDefaults
import androidx.compose.material3.SingleChoiceSegmentedButtonRow
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import com.example.tracer.data.ThemeMode
import com.example.tracer.ui.components.TracerSegmentedButtonDefaults

@Composable
internal fun ThemeModeSection(
    selectedThemeMode: ThemeMode,
    supportsLightDarkMode: Boolean = true,
    onSetThemeMode: (ThemeMode) -> Unit
) {
    Text(
        text = stringResource(R.string.config_title_theme_mode),
        style = MaterialTheme.typography.bodyMedium,
        color = MaterialTheme.colorScheme.onSurfaceVariant
    )

    if (!supportsLightDarkMode) {
        Text(
            text = stringResource(R.string.config_theme_mode_fixed_hint),
            style = MaterialTheme.typography.bodySmall,
            color = MaterialTheme.colorScheme.onSurfaceVariant
        )
        return
    }

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
