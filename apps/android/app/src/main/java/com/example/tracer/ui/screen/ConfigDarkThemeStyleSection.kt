package com.example.tracer

import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.SegmentedButton
import androidx.compose.material3.SegmentedButtonDefaults
import androidx.compose.material3.SingleChoiceSegmentedButtonRow
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import com.example.tracer.data.DarkSurfaceStyle

@Composable
internal fun DarkSurfaceStyleSection(
    selectedDarkSurfaceStyle: DarkSurfaceStyle,
    onSetDarkSurfaceStyle: (DarkSurfaceStyle) -> Unit
) {
    Text(
        text = stringResource(R.string.config_title_dark_surface_style),
        style = MaterialTheme.typography.titleMedium,
        color = MaterialTheme.colorScheme.onSurfaceVariant
    )

    var isExpanded by rememberSaveable { mutableStateOf(false) }
    val darkSurfaceStyles = DarkSurfaceStyle.entries
    ExpandableSettingsButton(
        text = darkSurfaceStyleLabel(selectedDarkSurfaceStyle),
        expanded = isExpanded,
        onClick = { isExpanded = !isExpanded }
    )
    if (isExpanded) {
        SingleChoiceSegmentedButtonRow(modifier = Modifier.fillMaxWidth()) {
            darkSurfaceStyles.forEachIndexed { index, style ->
                SegmentedButton(
                    shape = SegmentedButtonDefaults.itemShape(index = index, count = darkSurfaceStyles.size),
                    onClick = { onSetDarkSurfaceStyle(style) },
                    selected = selectedDarkSurfaceStyle == style,
                    modifier = Modifier.weight(1f),
                    label = {
                        Text(
                            text = darkSurfaceStyleLabel(style),
                            style = MaterialTheme.typography.labelSmall
                        )
                    }
                )
            }
        }
    }
}

@Composable
private fun darkSurfaceStyleLabel(style: DarkSurfaceStyle): String = when (style) {
    DarkSurfaceStyle.Neutral -> stringResource(R.string.config_dark_surface_style_neutral)
    DarkSurfaceStyle.Black -> stringResource(R.string.config_dark_surface_style_black)
}
