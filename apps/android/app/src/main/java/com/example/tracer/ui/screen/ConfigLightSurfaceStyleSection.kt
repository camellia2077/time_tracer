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
import com.example.tracer.data.LightSurfaceStyle

@Composable
internal fun LightSurfaceStyleSection(
    selectedLightSurfaceStyle: LightSurfaceStyle,
    onSetLightSurfaceStyle: (LightSurfaceStyle) -> Unit
) {
    Text(
        text = stringResource(R.string.config_title_light_surface_style),
        style = MaterialTheme.typography.titleMedium,
        color = MaterialTheme.colorScheme.onSurfaceVariant
    )

    val lightSurfaceStyles = LightSurfaceStyle.entries
    SingleChoiceSegmentedButtonRow(modifier = Modifier.fillMaxWidth()) {
        lightSurfaceStyles.forEachIndexed { index, style ->
            SegmentedButton(
                shape = SegmentedButtonDefaults.itemShape(index = index, count = lightSurfaceStyles.size),
                onClick = { onSetLightSurfaceStyle(style) },
                selected = selectedLightSurfaceStyle == style,
                modifier = Modifier.weight(1f),
                label = {
                    Text(
                        text = lightSurfaceStyleLabel(style),
                        style = MaterialTheme.typography.labelSmall
                    )
                }
            )
        }
    }
}

@Composable
private fun lightSurfaceStyleLabel(style: LightSurfaceStyle): String = when (style) {
    LightSurfaceStyle.Neutral -> stringResource(R.string.config_light_surface_style_neutral)
    LightSurfaceStyle.Elevated -> stringResource(R.string.config_light_surface_style_elevated)
}
