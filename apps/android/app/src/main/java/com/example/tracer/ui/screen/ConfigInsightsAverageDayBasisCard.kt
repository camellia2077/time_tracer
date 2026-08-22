package com.example.tracer

import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.TrendingDown
import androidx.compose.material.icons.automirrored.filled.TrendingUp
import androidx.compose.material.icons.filled.ArrowDownward
import androidx.compose.material.icons.filled.ArrowUpward
import androidx.compose.material.icons.filled.Add
import androidx.compose.material.icons.filled.Remove
import androidx.compose.material3.ElevatedCard
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.RadioButton
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.platform.testTag
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp

@Composable
internal fun ConfigInsightsAverageDayBasisCard(
    insightsPiePalettePreset: InsightsPiePalettePreset,
    onInsightsPiePalettePresetChange: (InsightsPiePalettePreset) -> Unit,
    comparisonColorScheme: InsightsComparisonColorScheme,
    onComparisonColorSchemeChange: (InsightsComparisonColorScheme) -> Unit,
    comparisonIndicatorStyle: InsightsComparisonIndicatorStyle,
    onComparisonIndicatorStyleChange: (InsightsComparisonIndicatorStyle) -> Unit,
    insightsChartStyleExpanded: Boolean,
    onInsightsChartStyleExpandedChange: (Boolean) -> Unit,
    insightsComparisonExpanded: Boolean,
    onInsightsComparisonExpandedChange: (Boolean) -> Unit,
    selected: InsightsAverageDayBasis,
    onSelected: (InsightsAverageDayBasis) -> Unit,
    expanded: Boolean = true,
    onToggleExpanded: () -> Unit = {}
) {
    ElevatedCard(modifier = Modifier.fillMaxWidth()) {
        Column(
            modifier = Modifier.padding(16.dp),
            verticalArrangement = androidx.compose.foundation.layout.Arrangement.spacedBy(12.dp)
        ) {
            ConfigCardHeader(
                title = stringResource(R.string.config_title_insights_settings),
                expanded = expanded,
                onToggleExpanded = onToggleExpanded
            )
            if (expanded) {
                InsightsChartStyleSection(
                    insightsPiePalettePreset = insightsPiePalettePreset,
                    onInsightsPiePalettePresetChange = onInsightsPiePalettePresetChange,
                    expanded = insightsChartStyleExpanded,
                    onToggleExpanded = {
                        onInsightsChartStyleExpandedChange(!insightsChartStyleExpanded)
                    }
                )
                androidx.compose.material3.HorizontalDivider()
                InsightsComparisonSection(
                    colorScheme = comparisonColorScheme,
                    onColorSchemeChange = onComparisonColorSchemeChange,
                    indicatorStyle = comparisonIndicatorStyle,
                    onIndicatorStyleChange = onComparisonIndicatorStyleChange,
                    expanded = insightsComparisonExpanded,
                    onToggleExpanded = {
                        onInsightsComparisonExpandedChange(!insightsComparisonExpanded)
                    }
                )
                androidx.compose.material3.HorizontalDivider()
                InsightsAverageDayBasisSection(selected = selected, onSelected = onSelected)
            }
        }
    }
}

@Composable
private fun InsightsAverageDayBasisSection(
    selected: InsightsAverageDayBasis,
    onSelected: (InsightsAverageDayBasis) -> Unit
) {
    Text(
        text = stringResource(R.string.config_title_insights_average_day_basis),
        style = MaterialTheme.typography.bodyMedium,
        color = MaterialTheme.colorScheme.onSurfaceVariant
    )
    var isExpanded by rememberSaveable { mutableStateOf(false) }
    ExpandableSettingsButton(
        text = averageDayBasisLabel(selected),
        expanded = isExpanded,
        onClick = { isExpanded = !isExpanded }
    )
    if (isExpanded) {
        Column(verticalArrangement = androidx.compose.foundation.layout.Arrangement.spacedBy(6.dp)) {
            Text(
                text = stringResource(R.string.config_insights_average_day_basis_description),
                style = MaterialTheme.typography.bodySmall,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
            InsightsAverageDayBasis.entries.forEach { basis ->
                Row {
                    RadioButton(selected = selected == basis, onClick = { onSelected(basis) })
                    Text(
                        text = averageDayBasisLabel(basis),
                        modifier = Modifier.padding(top = 12.dp)
                    )
                }
            }
        }
    }
}

@Composable
private fun InsightsComparisonSection(
    colorScheme: InsightsComparisonColorScheme,
    onColorSchemeChange: (InsightsComparisonColorScheme) -> Unit,
    indicatorStyle: InsightsComparisonIndicatorStyle,
    onIndicatorStyleChange: (InsightsComparisonIndicatorStyle) -> Unit,
    expanded: Boolean,
    onToggleExpanded: () -> Unit
) {
    Text(
        text = stringResource(R.string.config_title_insights_comparison),
        style = MaterialTheme.typography.bodyMedium,
        color = MaterialTheme.colorScheme.onSurfaceVariant
    )
    ExpandableSettingsButton(
        text = stringResource(
            R.string.config_insights_comparison_current,
            stringResource(colorScheme.labelRes()),
            stringResource(indicatorStyle.labelRes())
        ),
        expanded = expanded,
        onClick = onToggleExpanded,
        previewContent = {
            ComparisonPresentationPreview(
                colorScheme = colorScheme,
                indicatorStyle = indicatorStyle
            )
        }
    )
    if (expanded) {
        Column(
            modifier = Modifier.fillMaxWidth(),
            verticalArrangement = androidx.compose.foundation.layout.Arrangement.spacedBy(8.dp)
        ) {
            Text(
                text = stringResource(R.string.config_title_insights_comparison_colors),
                style = MaterialTheme.typography.labelSmall,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
            Text(
                text = stringResource(R.string.config_insights_comparison_colors_description),
                style = MaterialTheme.typography.bodySmall,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
            ComparisonPresentationExample(
                colorScheme = colorScheme,
                indicatorStyle = indicatorStyle
            )
            Column(verticalArrangement = androidx.compose.foundation.layout.Arrangement.spacedBy(6.dp)) {
                InsightsComparisonColorScheme.entries.forEach { scheme ->
                    ComparisonColorSchemeOption(
                        colorScheme = scheme,
                        indicatorStyle = indicatorStyle,
                        selected = colorScheme == scheme,
                        onClick = { onColorSchemeChange(scheme) }
                    )
                }
            }
            androidx.compose.material3.HorizontalDivider()
            Text(
                text = stringResource(R.string.config_title_insights_comparison_indicator),
                style = MaterialTheme.typography.labelSmall,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
            Column(verticalArrangement = androidx.compose.foundation.layout.Arrangement.spacedBy(6.dp)) {
                InsightsComparisonIndicatorStyle.entries.forEach { style ->
                    ComparisonIndicatorStyleOption(
                        indicatorStyle = style,
                        selected = indicatorStyle == style,
                        onClick = { onIndicatorStyleChange(style) }
                    )
                }
            }
        }
    }
}

@Composable
private fun ComparisonPresentationPreview(
    colorScheme: InsightsComparisonColorScheme,
    indicatorStyle: InsightsComparisonIndicatorStyle
) {
    val (increase, decrease) = comparisonPreviewColors(colorScheme)
    val (increaseIcon, decreaseIcon) = comparisonIndicatorIcons(indicatorStyle)
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .testTag("config_insights_comparison_summary_preview"),
        horizontalArrangement = androidx.compose.foundation.layout.Arrangement.spacedBy(12.dp)
    ) {
        ComparisonColorPreviewItem(
            icon = increaseIcon,
            label = stringResource(R.string.config_insights_comparison_preview_increase),
            color = increase,
            modifier = Modifier.weight(1f)
        )
        ComparisonColorPreviewItem(
            icon = decreaseIcon,
            label = stringResource(R.string.config_insights_comparison_preview_decrease),
            color = decrease,
            modifier = Modifier.weight(1f)
        )
    }
}

@Composable
private fun ComparisonPresentationExample(
    colorScheme: InsightsComparisonColorScheme,
    indicatorStyle: InsightsComparisonIndicatorStyle
) {
    val (increase, decrease) = comparisonPreviewColors(colorScheme)
    val (increaseIcon, decreaseIcon) = comparisonIndicatorIcons(indicatorStyle)
    Surface(
        modifier = Modifier
            .fillMaxWidth()
            .testTag("config_insights_comparison_presentation_example"),
        color = MaterialTheme.colorScheme.surfaceContainerLow,
        shape = MaterialTheme.shapes.small
    ) {
        Column(modifier = Modifier.padding(12.dp)) {
            Text(
                text = stringResource(R.string.config_title_insights_comparison_example),
                style = MaterialTheme.typography.labelMedium,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
            Spacer(modifier = Modifier.size(6.dp))
            ComparisonExampleLine(
                icon = increaseIcon,
                color = increase,
                text = stringResource(R.string.config_insights_comparison_example_increase)
            )
            ComparisonExampleLine(
                icon = decreaseIcon,
                color = decrease,
                text = stringResource(R.string.config_insights_comparison_example_decrease)
            )
        }
    }
}

@Composable
private fun ComparisonExampleLine(icon: ImageVector, color: Color, text: String) {
    Row {
        Icon(imageVector = icon, contentDescription = null, tint = color)
        Spacer(modifier = Modifier.size(8.dp))
        Text(text = text, style = MaterialTheme.typography.bodySmall, color = color)
    }
}

@Composable
private fun ComparisonIndicatorStyleOption(
    indicatorStyle: InsightsComparisonIndicatorStyle,
    selected: Boolean,
    onClick: () -> Unit
) {
    val (increaseIcon, decreaseIcon) = comparisonIndicatorIcons(indicatorStyle)
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .testTag("config_insights_comparison_indicator_option")
    ) {
        RadioButton(selected = selected, onClick = onClick)
        Row(modifier = Modifier.padding(top = 8.dp)) {
            Icon(
                imageVector = increaseIcon,
                contentDescription = null,
                tint = MaterialTheme.colorScheme.onSurfaceVariant
            )
            Spacer(modifier = Modifier.size(8.dp))
            Icon(
                imageVector = decreaseIcon,
                contentDescription = null,
                tint = MaterialTheme.colorScheme.onSurfaceVariant
            )
            Spacer(modifier = Modifier.size(12.dp))
            Text(
                text = stringResource(indicatorStyle.labelRes()),
                style = MaterialTheme.typography.bodyMedium
            )
        }
    }
}

@Composable
private fun ComparisonColorSchemeOption(
    colorScheme: InsightsComparisonColorScheme,
    indicatorStyle: InsightsComparisonIndicatorStyle,
    selected: Boolean,
    onClick: () -> Unit
) {
    val (increase, decrease) = comparisonPreviewColors(colorScheme)
    val (increaseIcon, decreaseIcon) = comparisonIndicatorIcons(indicatorStyle)
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .testTag("config_insights_comparison_color_option")
    ) {
        RadioButton(selected = selected, onClick = onClick)
        Column(modifier = Modifier.padding(top = 6.dp)) {
            Text(
                text = stringResource(colorScheme.labelRes()),
                style = MaterialTheme.typography.bodyMedium
            )
            Row(modifier = Modifier.padding(top = 4.dp)) {
            ComparisonColorPreviewItem(
                icon = increaseIcon,
                color = increase,
                label = stringResource(R.string.config_insights_comparison_preview_increase),
                modifier = Modifier.weight(1f)
            )
                Spacer(modifier = Modifier.size(12.dp))
            ComparisonColorPreviewItem(
                icon = decreaseIcon,
                color = decrease,
                label = stringResource(R.string.config_insights_comparison_preview_decrease),
                modifier = Modifier.weight(1f)
            )
            }
        }
    }
}

@Composable
private fun ComparisonColorPreviewItem(
    icon: ImageVector,
    color: Color,
    label: String,
    modifier: Modifier = Modifier
) {
    Row(modifier = modifier) {
        Icon(imageVector = icon, contentDescription = null, tint = color)
        Spacer(modifier = Modifier.size(6.dp))
        Surface(modifier = Modifier.size(12.dp), color = color, shape = androidx.compose.foundation.shape.CircleShape) {}
        Spacer(modifier = Modifier.size(6.dp))
        Text(text = label, style = MaterialTheme.typography.labelMedium)
    }
}

@Composable
private fun comparisonPreviewColors(colorScheme: InsightsComparisonColorScheme): Pair<Color, Color> =
    when (colorScheme) {
        InsightsComparisonColorScheme.GREEN_RED -> Color(0xFF1A7F37) to Color(0xFFCF222E)
        InsightsComparisonColorScheme.RED_GREEN -> Color(0xFFCF222E) to Color(0xFF1A7F37)
        InsightsComparisonColorScheme.THEME_ACCENT_NEUTRAL ->
            MaterialTheme.colorScheme.secondary to Color(0xFF9CA3AF)
        InsightsComparisonColorScheme.BLUE_ORANGE -> Color(0xFF2563EB) to Color(0xFFF97316)
    }

private fun InsightsComparisonColorScheme.labelRes(): Int = when (this) {
    InsightsComparisonColorScheme.GREEN_RED -> R.string.config_insights_comparison_colors_green_red
    InsightsComparisonColorScheme.RED_GREEN -> R.string.config_insights_comparison_colors_red_green
    InsightsComparisonColorScheme.THEME_ACCENT_NEUTRAL ->
        R.string.config_insights_comparison_colors_theme_neutral
    InsightsComparisonColorScheme.BLUE_ORANGE -> R.string.config_insights_comparison_colors_blue_orange
}

private fun InsightsComparisonIndicatorStyle.labelRes(): Int = when (this) {
    InsightsComparisonIndicatorStyle.ARROWS -> R.string.config_insights_comparison_indicator_arrows
    InsightsComparisonIndicatorStyle.TREND_LINES ->
        R.string.config_insights_comparison_indicator_trend_lines
    InsightsComparisonIndicatorStyle.SIGNS -> R.string.config_insights_comparison_indicator_signs
}

@Composable
private fun averageDayBasisLabel(basis: InsightsAverageDayBasis): String = stringResource(
    if (basis == InsightsAverageDayBasis.ACTIVE_DAYS) {
        R.string.config_insights_average_day_basis_active
    } else {
        R.string.config_insights_average_day_basis_calendar
    }
)

private fun comparisonIndicatorIcons(
    indicatorStyle: InsightsComparisonIndicatorStyle
): Pair<ImageVector, ImageVector> = when (indicatorStyle) {
    InsightsComparisonIndicatorStyle.ARROWS ->
        Icons.Filled.ArrowUpward to Icons.Filled.ArrowDownward
    InsightsComparisonIndicatorStyle.TREND_LINES ->
        Icons.AutoMirrored.Filled.TrendingUp to Icons.AutoMirrored.Filled.TrendingDown
    InsightsComparisonIndicatorStyle.SIGNS -> Icons.Filled.Add to Icons.Filled.Remove
}
