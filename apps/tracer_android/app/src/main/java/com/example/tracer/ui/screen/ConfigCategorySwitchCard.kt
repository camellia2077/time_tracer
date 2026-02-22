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

@OptIn(ExperimentalMaterial3Api::class)
@Composable
internal fun ConfigCategorySwitchCard(
    selectedCategory: ConfigCategory,
    selectedFile: String,
    visibleFiles: List<String>,
    onSelectConverter: () -> Unit,
    onSelectReports: () -> Unit,
    onRefreshFiles: () -> Unit,
    onOpenFile: (String) -> Unit,
    onImportAndroidConfigFullReplace: () -> Unit,
    onImportAndroidConfigPartialUpdate: () -> Unit,
    onExportAndroidConfig: () -> Unit
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

            OutlinedButton(
                onClick = onImportAndroidConfigFullReplace,
                modifier = Modifier.fillMaxWidth()
            ) {
                Text(stringResource(R.string.config_action_import_android_config_full_replace))
            }

            OutlinedButton(
                onClick = onImportAndroidConfigPartialUpdate,
                modifier = Modifier.fillMaxWidth()
            ) {
                Text(stringResource(R.string.config_action_import_android_config_partial_update))
            }

            OutlinedButton(
                onClick = onExportAndroidConfig,
                modifier = Modifier.fillMaxWidth()
            ) {
                Text(stringResource(R.string.config_action_export_android_config))
            }

            val configCategories = listOf(
                ConfigCategory.CONVERTER to stringResource(R.string.config_category_converter),
                ConfigCategory.REPORTS to stringResource(R.string.config_category_reports)
            )
            SingleChoiceSegmentedButtonRow(modifier = Modifier.fillMaxWidth()) {
                configCategories.forEachIndexed { index, (category, label) ->
                    SegmentedButton(
                        shape = SegmentedButtonDefaults.itemShape(index = index, count = configCategories.size),
                        onClick = {
                            if (category == ConfigCategory.CONVERTER) {
                                onSelectConverter()
                            } else {
                                onSelectReports()
                            }
                        },
                        selected = selectedCategory == category,
                        modifier = Modifier.weight(1f),
                        label = { Text(label) }
                    )
                }
            }

            if (visibleFiles.isEmpty()) {
                val categoryLabel = when (selectedCategory) {
                    ConfigCategory.CONVERTER -> stringResource(R.string.config_category_converter)
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
                        val buttonText = if (selectedFile.isNotBlank()) {
                            selectedFile
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
                        for (filePath in visibleFiles) {
                            DropdownMenuItem(
                                text = { Text(filePath) },
                                onClick = {
                                    fileMenuExpanded = false
                                    onOpenFile(filePath)
                                }
                            )
                        }
                    }
                }
            }
        }
    }
}
