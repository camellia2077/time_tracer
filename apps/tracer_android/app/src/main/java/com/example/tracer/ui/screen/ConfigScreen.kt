package com.example.tracer

import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.text.selection.SelectionContainer
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ArrowDropDown
import androidx.compose.material.icons.filled.Check
import androidx.compose.material.icons.filled.Refresh
import androidx.compose.material3.Button
import androidx.compose.material3.DropdownMenu
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.ElevatedCard
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp
import androidx.compose.foundation.horizontalScroll
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.layout.FlowRow
import androidx.compose.foundation.layout.ExperimentalLayoutApi

private const val ABOUT_AUTHOR = "camellia2077"
private const val ABOUT_REPOSITORY = "https://github.com/camellia2077/time_tracer_cpp"
private const val ABOUT_CORE_VERSION = "0.6.1"
private const val ABOUT_ANDROID_APP_VERSION = "0.1.0"

@Composable
fun ConfigSection(
    selectedCategory: ConfigCategory,
    converterFiles: List<String>,
    reportFiles: List<String>,
    selectedFile: String,
    editableContent: String,
    themeConfig: com.example.tracer.data.ThemeConfig,
    onSelectConverter: () -> Unit,
    onSelectReports: () -> Unit,
    onRefreshFiles: () -> Unit,
    onOpenFile: (String) -> Unit,
    onEditableContentChange: (String) -> Unit,
    onSaveCurrentFile: () -> Unit,
    onSetThemeColor: (com.example.tracer.data.ThemeColor) -> Unit,
    onSetThemeMode: (com.example.tracer.data.ThemeMode) -> Unit,
    onSetUseDynamicColor: (Boolean) -> Unit,
    onSetDarkThemeStyle: (com.example.tracer.data.DarkThemeStyle) -> Unit
) {
    var fileMenuExpanded by remember { mutableStateOf(false) }

    val visibleFiles = when (selectedCategory) {
        ConfigCategory.CONVERTER -> converterFiles
        ConfigCategory.REPORTS -> reportFiles
    }

    Column(
        modifier = Modifier.fillMaxWidth(),
        verticalArrangement = Arrangement.spacedBy(16.dp)
    ) {
        // Appearance Card
        ElevatedCard(modifier = Modifier.fillMaxWidth()) {
            Column(
                modifier = Modifier.padding(16.dp),
                verticalArrangement = Arrangement.spacedBy(12.dp)
            ) {
                Text(
                    text = "Appearance",
                    style = MaterialTheme.typography.titleMedium,
                    color = MaterialTheme.colorScheme.primary
                )

                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.SpaceBetween,
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Text(
                        text = "Use Wallpaper Colors",
                        style = MaterialTheme.typography.bodyMedium,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                    androidx.compose.material3.Switch(
                        checked = themeConfig.useDynamicColor,
                        onCheckedChange = onSetUseDynamicColor
                    )
                }

                if (!themeConfig.useDynamicColor) {
                    Text(
                        text = "Theme Color",
                        style = MaterialTheme.typography.bodyMedium,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )

                    @OptIn(ExperimentalLayoutApi::class)
                    FlowRow(
                        modifier = Modifier.fillMaxWidth(),
                        horizontalArrangement = Arrangement.spacedBy(8.dp),
                        verticalArrangement = Arrangement.spacedBy(8.dp)
                    ) {
                        com.example.tracer.data.ThemeColor.entries.forEach { color ->
                            val isSelected = themeConfig.themeColor == color
                            val colorValue = when (color) {
                                com.example.tracer.data.ThemeColor.Rose -> com.example.tracer.ui.theme.Rose500
                                com.example.tracer.data.ThemeColor.Orange -> com.example.tracer.ui.theme.Orange500
                                com.example.tracer.data.ThemeColor.Peach -> com.example.tracer.ui.theme.Peach500
                                com.example.tracer.data.ThemeColor.Gold -> com.example.tracer.ui.theme.Gold500
                                com.example.tracer.data.ThemeColor.Mint -> com.example.tracer.ui.theme.Mint500
                                com.example.tracer.data.ThemeColor.Emerald -> com.example.tracer.ui.theme.Emerald500
                                com.example.tracer.data.ThemeColor.Teal -> com.example.tracer.ui.theme.Teal500
                                com.example.tracer.data.ThemeColor.Cyan -> com.example.tracer.ui.theme.Cyan500
                                com.example.tracer.data.ThemeColor.Sky -> com.example.tracer.ui.theme.Sky500
                                com.example.tracer.data.ThemeColor.Lavender -> com.example.tracer.ui.theme.Lavender500
                                com.example.tracer.data.ThemeColor.Violet -> com.example.tracer.ui.theme.Violet500
                                com.example.tracer.data.ThemeColor.Pink -> com.example.tracer.ui.theme.Pink500
                                com.example.tracer.data.ThemeColor.Sakura -> com.example.tracer.ui.theme.Sakura500
                                com.example.tracer.data.ThemeColor.Magenta -> com.example.tracer.ui.theme.Magenta500
                                com.example.tracer.data.ThemeColor.Slate -> com.example.tracer.ui.theme.Indigo500
                            }

                            Box(
                                modifier = Modifier
                                    .width(32.dp)
                                    .height(48.dp)
                                    .background(colorValue, CircleShape)
                                    .border(
                                        width = if (isSelected) 3.dp else 0.dp,
                                        color = if (isSelected) MaterialTheme.colorScheme.primary else Color.Transparent,
                                        shape = CircleShape
                                    )
                                    .clickable { onSetThemeColor(color) },
                                contentAlignment = Alignment.Center
                            ) {
                                if (isSelected) {
                                    Icon(
                                        imageVector = Icons.Filled.Check,
                                        contentDescription = null,
                                        tint = Color.White,
                                        modifier = Modifier.size(16.dp)
                                    )
                                }
                            }
                        }
                    }
                }

                HorizontalDivider()

                Text(
                    text = "Theme Mode",
                    style = MaterialTheme.typography.bodyMedium,
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )

                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.spacedBy(8.dp)
                ) {
                    com.example.tracer.data.ThemeMode.entries.forEach { mode ->
                        val isSelected = themeConfig.themeMode == mode
                        OutlinedButton(
                            onClick = { onSetThemeMode(mode) },
                            modifier = Modifier.weight(1f),
                            colors = if (isSelected) {
                                androidx.compose.material3.ButtonDefaults.outlinedButtonColors(
                                    containerColor = MaterialTheme.colorScheme.primaryContainer,
                                    contentColor = MaterialTheme.colorScheme.onPrimaryContainer
                                )
                            } else {
                                androidx.compose.material3.ButtonDefaults.outlinedButtonColors()
                            }
                        ) {
                            Text(mode.name)
                        }
                    }
                }

                // Dark Theme Style (Visible only if Dark Mode is active or System is Dark)
                val isSystemDark = androidx.compose.foundation.isSystemInDarkTheme()
                val isDarkActive = themeConfig.themeMode == com.example.tracer.data.ThemeMode.Dark || 
                                  (themeConfig.themeMode == com.example.tracer.data.ThemeMode.System && isSystemDark)

                if (isDarkActive) {
                    HorizontalDivider()
                    
                    Text(
                        text = "Dark Theme Style",
                        style = MaterialTheme.typography.bodyMedium,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )

                    Row(
                        modifier = Modifier.fillMaxWidth(),
                        horizontalArrangement = Arrangement.spacedBy(8.dp)
                    ) {
                        com.example.tracer.data.DarkThemeStyle.entries.forEach { style ->
                            val isSelected = themeConfig.darkThemeStyle == style
                            OutlinedButton(
                                onClick = { onSetDarkThemeStyle(style) },
                                modifier = Modifier.weight(1f),
                                colors = if (isSelected) {
                                    androidx.compose.material3.ButtonDefaults.outlinedButtonColors(
                                        containerColor = MaterialTheme.colorScheme.primaryContainer,
                                        contentColor = MaterialTheme.colorScheme.onPrimaryContainer
                                    )
                                } else {
                                    androidx.compose.material3.ButtonDefaults.outlinedButtonColors()
                                }
                            ) {
                                Text(
                                    text = when(style) {
                                        com.example.tracer.data.DarkThemeStyle.Tinted -> "Tinted"
                                        com.example.tracer.data.DarkThemeStyle.Neutral -> "Grey"
                                        com.example.tracer.data.DarkThemeStyle.Black -> "Black"
                                    },
                                    style = MaterialTheme.typography.labelSmall
                                )
                            }
                        }
                    }
                }
            }
        }

        // File Selection Card
        ElevatedCard(modifier = Modifier.fillMaxWidth()) {
            Column(
                modifier = Modifier.padding(16.dp),
                verticalArrangement = Arrangement.spacedBy(12.dp)
            ) {
                Text(
                    text = "Configuration Files",
                    style = MaterialTheme.typography.titleMedium,
                    color = MaterialTheme.colorScheme.primary
                )

                OutlinedButton(
                    onClick = onRefreshFiles,
                    modifier = Modifier.fillMaxWidth()
                ) {
                    Icon(Icons.Filled.Refresh, contentDescription = null)
                    Spacer(modifier = Modifier.width(8.dp))
                    Text("Refresh List")
                }

                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.spacedBy(8.dp)
                ) {
                    Button(
                        onClick = onSelectConverter,
                        enabled = selectedCategory != ConfigCategory.CONVERTER,
                        modifier = Modifier.weight(1f)
                    ) {
                        Text("Converter")
                    }

                    Button(
                        onClick = onSelectReports,
                        enabled = selectedCategory != ConfigCategory.REPORTS,
                        modifier = Modifier.weight(1f)
                    ) {
                        Text("Reports")
                    }
                }

                if (visibleFiles.isEmpty()) {
                    Text(
                        text = "No TOML files in ${selectedCategory.name.lowercase()}.",
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
                                "Select TOML file"
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

        if (visibleFiles.isNotEmpty()) {
            // Editor Card
            ElevatedCard(modifier = Modifier.fillMaxWidth()) {
                Column(
                    modifier = Modifier.padding(16.dp),
                    verticalArrangement = Arrangement.spacedBy(12.dp)
                ) {
                    Text(
                        text = "Editor: $selectedFile",
                        style = MaterialTheme.typography.titleMedium,
                        color = MaterialTheme.colorScheme.primary
                    )

                    OutlinedTextField(
                        value = editableContent,
                        onValueChange = onEditableContentChange,
                        label = { Text("TOML Content") },
                        modifier = Modifier.fillMaxWidth(),
                        minLines = 12,
                        textStyle = MaterialTheme.typography.bodyMedium.copy(fontFamily = androidx.compose.ui.text.font.FontFamily.Monospace)
                    )

                    Button(
                        onClick = onSaveCurrentFile,
                        modifier = Modifier.fillMaxWidth()
                    ) {
                        Icon(Icons.Filled.Check, contentDescription = null)
                        Spacer(modifier = Modifier.width(8.dp))
                        Text("Save Changes")
                    }
                }
            }
        }

        // About Card
        ElevatedCard(modifier = Modifier.fillMaxWidth()) {
            Column(
                modifier = Modifier.padding(16.dp),
                verticalArrangement = Arrangement.spacedBy(8.dp)
            ) {
                Text(
                    text = "About",
                    style = MaterialTheme.typography.titleMedium,
                    color = MaterialTheme.colorScheme.primary
                )

                SelectionContainer {
                    Column(verticalArrangement = Arrangement.spacedBy(4.dp)) {
                        Text("Author: $ABOUT_AUTHOR", style = MaterialTheme.typography.bodyMedium)
                        Text("Repo: $ABOUT_REPOSITORY", style = MaterialTheme.typography.bodyMedium)
                        HorizontalDivider()
                        Text("Core: $ABOUT_CORE_VERSION", style = MaterialTheme.typography.bodySmall)
                        Text("App: $ABOUT_ANDROID_APP_VERSION", style = MaterialTheme.typography.bodySmall)
                    }
                }
            }
        }
    }
}
