package com.example.tracer

import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.ElevatedCard
import androidx.compose.material3.RadioButton
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp

@Composable
internal fun ConfigInsightsAverageDayBasisCard(
    insightsPiePalettePreset: InsightsPiePalettePreset,
    onInsightsPiePalettePresetChange: (InsightsPiePalettePreset) -> Unit,
    initialInsightsPaletteExpanded: Boolean = false,
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
                    initialExpanded = initialInsightsPaletteExpanded
                )
                androidx.compose.material3.HorizontalDivider()
                Text(
                    text = stringResource(R.string.config_title_insights_average_day_basis),
                    style = androidx.compose.material3.MaterialTheme.typography.bodyMedium,
                    color = androidx.compose.material3.MaterialTheme.colorScheme.onSurfaceVariant
                )
                Text(
                    text = stringResource(R.string.config_insights_average_day_basis_description),
                    style = androidx.compose.material3.MaterialTheme.typography.bodySmall,
                    color = androidx.compose.material3.MaterialTheme.colorScheme.onSurfaceVariant
                )
                InsightsAverageDayBasis.entries.forEach { basis ->
                    androidx.compose.foundation.layout.Row {
                        RadioButton(selected = selected == basis, onClick = { onSelected(basis) })
                        Text(
                            text = stringResource(
                                if (basis == InsightsAverageDayBasis.ACTIVE_DAYS) {
                                    R.string.config_insights_average_day_basis_active
                                } else {
                                    R.string.config_insights_average_day_basis_calendar
                                }
                            ),
                            modifier = Modifier.padding(top = 12.dp)
                        )
                    }
                }
            }
        }
    }
}
