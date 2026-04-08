package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ArrowDropDown
import androidx.compose.material.icons.filled.Refresh
import androidx.compose.material3.DropdownMenu
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.ElevatedCard
import androidx.compose.material3.ExperimentalMaterial3Api
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
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import com.example.tracer.ui.components.TracerSegmentedButtonDefaults

@OptIn(ExperimentalMaterial3Api::class)
@Composable
internal fun ConfigCategorySwitchCard(
    selectedCategory: ConfigCategory,
    selectedConverterSubcategory: ConverterSubcategory,
    selectedFileDisplayName: String,
    visibleFiles: List<ConfigTomlFileEntry>,
    onSelectConverter: () -> Unit,
    onSelectCharts: () -> Unit,
    onSelectMeta: () -> Unit,
    onSelectReports: () -> Unit,
    onSelectConverterAliases: () -> Unit,
    onSelectConverterRules: () -> Unit,
    onRefreshFiles: () -> Unit,
    onOpenFile: (String) -> Unit
) {
    var fileMenuExpanded by remember { mutableStateOf(false) }

    ElevatedCard(modifier = Modifier.fillMaxWidth()) {
        Column(
            modifier = Modifier.padding(16.dp),
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
                ConfigCategory.CONVERTER to stringResource(R.string.config_category_converter),
                ConfigCategory.CHARTS to stringResource(R.string.config_category_charts),
                ConfigCategory.META to stringResource(R.string.config_category_meta),
                ConfigCategory.REPORTS to stringResource(R.string.config_category_reports)
            )
            SingleChoiceSegmentedButtonRow(modifier = Modifier.fillMaxWidth()) {
                configCategories.forEachIndexed { index, (category, label) ->
                    val selected = selectedCategory == category
                    SegmentedButton(
                        shape = SegmentedButtonDefaults.itemShape(index = index, count = configCategories.size),
                        onClick = {
                            when (category) {
                                ConfigCategory.CONVERTER -> onSelectConverter()
                                ConfigCategory.CHARTS -> onSelectCharts()
                                ConfigCategory.META -> onSelectMeta()
                                ConfigCategory.REPORTS -> onSelectReports()
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

            if (selectedCategory == ConfigCategory.CONVERTER) {
                val converterSections = listOf(
                    ConverterSubcategory.ALIASES to stringResource(R.string.config_converter_section_aliases),
                    ConverterSubcategory.RULES to stringResource(R.string.config_converter_section_rules)
                )
                SingleChoiceSegmentedButtonRow(modifier = Modifier.fillMaxWidth()) {
                    converterSections.forEachIndexed { index, (subcategory, label) ->
                        val selected = selectedConverterSubcategory == subcategory
                        SegmentedButton(
                            shape = SegmentedButtonDefaults.itemShape(
                                index = index,
                                count = converterSections.size
                            ),
                            onClick = {
                                when (subcategory) {
                                    ConverterSubcategory.ALIASES -> onSelectConverterAliases()
                                    ConverterSubcategory.RULES -> onSelectConverterRules()
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

            if (visibleFiles.isEmpty()) {
                val categoryLabel = when (selectedCategory) {
                    ConfigCategory.CONVERTER -> stringResource(R.string.config_category_converter)
                    ConfigCategory.CHARTS -> stringResource(R.string.config_category_charts)
                    ConfigCategory.META -> stringResource(R.string.config_category_meta)
                    ConfigCategory.REPORTS -> stringResource(R.string.config_category_reports)
                }
                Text(
                    text = stringResource(R.string.config_no_toml_files_in_category, categoryLabel),
                    style = MaterialTheme.typography.bodySmall,
                    modifier = Modifier.fillMaxWidth()
                )
            } else {
                Box(modifier = Modifier.fillMaxWidth()) {
                    OutlinedButton(
                        onClick = { fileMenuExpanded = true },
                        modifier = Modifier.fillMaxWidth()
                    ) {
                        val buttonText = if (selectedFileDisplayName.isNotBlank()) {
                            selectedFileDisplayName
                        } else {
                            stringResource(R.string.config_action_select_toml_file)
                        }
                        Text(buttonText)
                        Spacer(modifier = Modifier.weight(1f))
                        Icon(Icons.Filled.ArrowDropDown, contentDescription = null)
                    }

                    DropdownMenu(
                        expanded = fileMenuExpanded,
                        onDismissRequest = { fileMenuExpanded = false }
                    ) {
                        for (fileEntry in visibleFiles) {
                            DropdownMenuItem(
                                text = { Text(fileEntry.displayName) },
                                onClick = {
                                    fileMenuExpanded = false
                                    onOpenFile(fileEntry.relativePath)
                                }
                            )
                        }
                    }
                }
            }
        }
    }
}
