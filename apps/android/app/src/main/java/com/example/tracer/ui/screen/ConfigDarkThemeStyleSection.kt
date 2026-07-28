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
import com.example.tracer.data.DarkThemeStyle

@Composable
internal fun DarkThemeStyleSection(
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
