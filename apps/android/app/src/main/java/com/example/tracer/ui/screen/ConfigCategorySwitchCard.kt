package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Refresh
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.SegmentedButton
import androidx.compose.material3.SegmentedButtonDefaults
import androidx.compose.material3.SingleChoiceSegmentedButtonRow
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import com.example.tracer.ui.components.TracerSegmentedButtonDefaults

@OptIn(ExperimentalMaterial3Api::class)
@Composable
internal fun ConfigCategorySwitchCard(
    selectedCategory: ConfigCategory,
    onSelectAlias: () -> Unit,
    onSelectCharts: () -> Unit,
    onSelectMeta: () -> Unit,
    onSelectInsights: () -> Unit,
    onRefreshFiles: () -> Unit
) {
    Column(
        modifier = Modifier
            .fillMaxWidth()
            .padding(16.dp),
        verticalArrangement = Arrangement.spacedBy(12.dp)
    ) {
            Text(
                text = stringResource(R.string.config_title_configuration_files),
                style = MaterialTheme.typography.titleMedium,
                color = MaterialTheme.colorScheme.primary
            )

            OutlinedButton(
                onClick = onRefreshFiles,
                modifier = Modifier.fillMaxWidth()
            ) {
                Icon(Icons.Filled.Refresh, contentDescription = null)
                Spacer(modifier = Modifier.width(8.dp))
                Text(stringResource(R.string.config_action_refresh_list))
            }

            val configCategories = listOf(
                ConfigCategory.ALIAS to stringResource(R.string.config_category_alias),
                ConfigCategory.CHARTS to stringResource(R.string.config_category_charts),
                ConfigCategory.META to stringResource(R.string.config_category_meta),
                ConfigCategory.INSIGHTS to stringResource(R.string.config_category_insights)
            )
            SingleChoiceSegmentedButtonRow(modifier = Modifier.fillMaxWidth()) {
                configCategories.forEachIndexed { index, (category, label) ->
                    val selected = selectedCategory == category
                    SegmentedButton(
                        shape = SegmentedButtonDefaults.itemShape(index = index, count = configCategories.size),
                        onClick = {
                            when (category) {
                                ConfigCategory.ALIAS -> onSelectAlias()
                                ConfigCategory.CHARTS -> onSelectCharts()
                                ConfigCategory.META -> onSelectMeta()
                                ConfigCategory.INSIGHTS -> onSelectInsights()
                            }
                        },
                        selected = selected,
                        modifier = Modifier.weight(1f),
                        colors = TracerSegmentedButtonDefaults.colors(),
                        label = {
                            Text(
                                text = label,
                                fontWeight = if (selected) {
                                    TracerSegmentedButtonDefaults.activeLabelFontWeight
                                } else {
                                    TracerSegmentedButtonDefaults.inactiveLabelFontWeight
                                }
                            )
                        }
                    )
                }
            }

    }
}
