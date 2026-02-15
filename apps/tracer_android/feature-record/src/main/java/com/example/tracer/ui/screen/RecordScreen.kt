package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.material3.Button
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Check
import androidx.compose.material.icons.filled.DateRange
import androidx.compose.material.icons.filled.DateRange
import androidx.compose.material.icons.filled.Edit
import androidx.compose.material.icons.filled.Settings
import androidx.compose.material.icons.filled.KeyboardArrowUp
import androidx.compose.material.icons.filled.KeyboardArrowDown
import androidx.compose.material3.ElevatedCard
import androidx.compose.material3.Icon
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.input.ImeAction
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.unit.dp
import kotlinx.coroutines.delay
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale

private val DISPLAY_TIME_FORMATTER = SimpleDateFormat("yyyy-MM-dd HH:mm:ss", Locale.US)
private const val MAX_QUICK_ACTIVITY_COUNT = 12

@Composable
fun RecordSection(
    recordContent: String,
    onRecordContentChange: (String) -> Unit,
    recordRemark: String,
    onRecordRemarkChange: (String) -> Unit,
    quickActivities: List<String>,
    availableActivityNames: List<String>,
    onQuickActivitiesUpdate: (List<String>) -> Unit,
    assistExpanded: Boolean,
    assistSettingsExpanded: Boolean,
    onToggleAssist: () -> Unit,
    onToggleAssistSettings: () -> Unit,
    suggestionLookbackDays: Int,
    suggestionTopN: Int,
    onSuggestionLookbackDaysChange: (String) -> Unit,
    onSuggestionTopNChange: (String) -> Unit,
    suggestedActivities: List<String>,
    suggestionsVisible: Boolean,
    isSuggestionsLoading: Boolean,
    useManualDate: Boolean,
    manualDate: String,
    onUseAutoDate: () -> Unit,
    onUseManualDate: () -> Unit,
    onManualDateChange: (String) -> Unit,
    onToggleSuggestions: () -> Unit,
    onSuggestedActivityClick: (String) -> Unit,
    onRecordNow: () -> Unit
) {
    var currentTimeText by remember { mutableStateOf(formatCurrentTime()) }
    var quickActivitySearch by remember { mutableStateOf("") }

    LaunchedEffect(Unit) {
        while (true) {
            currentTimeText = formatCurrentTime()
            delay(1000)
        }
    }

    Column(
        modifier = Modifier.fillMaxWidth(),
        verticalArrangement = Arrangement.spacedBy(16.dp)
    ) {
        // Compact Time Settings
        ElevatedCard(modifier = Modifier.fillMaxWidth()) {
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(16.dp),
                verticalAlignment = androidx.compose.ui.Alignment.CenterVertically,
                horizontalArrangement = Arrangement.SpaceBetween
            ) {
                Column(verticalArrangement = Arrangement.spacedBy(4.dp)) {
                    Text(
                        text = "Target Date",
                        style = MaterialTheme.typography.labelMedium,
                        color = MaterialTheme.colorScheme.primary
                    )
                    Text(
                        text = if (useManualDate) manualDate else currentTimeText.substring(0, 10),
                        style = MaterialTheme.typography.titleMedium
                    )
                }

                Row(verticalAlignment = androidx.compose.ui.Alignment.CenterVertically) {
                    Text(
                        text = if (useManualDate) "Manual" else "Auto",
                        style = MaterialTheme.typography.labelSmall,
                        modifier = Modifier.padding(end = 8.dp)
                    )
                    androidx.compose.material3.Switch(
                        checked = useManualDate,
                        onCheckedChange = { if (it) onUseManualDate() else onUseAutoDate() }
                    )
                }
            }
            if (useManualDate) {
               androidx.compose.material3.HorizontalDivider()
                Row(
                    modifier = Modifier.padding(16.dp)
                ) {
                    com.example.tracer.ui.components.SegmentedDateInput(
                        value = manualDate,
                        onValueChange = onManualDateChange,
                        modifier = Modifier.fillMaxWidth()
                    )
                }
            }
        }

        // Main Record Input Card
        ElevatedCard(modifier = Modifier.fillMaxWidth()) {
            Column(
                modifier = Modifier.padding(16.dp),
                verticalArrangement = Arrangement.spacedBy(12.dp)
            ) {
                Text(
                    text = "Record Input",
                    style = MaterialTheme.typography.titleMedium,
                    color = MaterialTheme.colorScheme.primary
                )

                OutlinedTextField(
                    value = recordContent,
                    onValueChange = onRecordContentChange,
                    label = { Text("Activity Name") },
                    leadingIcon = { Icon(Icons.Default.Edit, contentDescription = null) },
                    singleLine = true,
                    modifier = Modifier.fillMaxWidth()
                )

                OutlinedTextField(
                    value = recordRemark,
                    onValueChange = onRecordRemarkChange,
                    label = { Text("Remark (Optional)") },
                    leadingIcon = { Icon(Icons.Default.Edit, contentDescription = null) },
                    singleLine = true,
                    modifier = Modifier.fillMaxWidth()
                )

                Button(
                    onClick = onRecordNow,
                    modifier = Modifier.fillMaxWidth().height(56.dp)
                ) {
                    Icon(Icons.Default.Check, contentDescription = null)
                    Spacer(modifier = Modifier.width(8.dp))
                    Text("Record Activity")
                }
            }
        }

        // Quick Activities & Suggestions
        Column(verticalArrangement = Arrangement.spacedBy(16.dp)) {
            // Quick Activities Header & Config
            Row(
                modifier = Modifier.fillMaxWidth(),
                verticalAlignment = androidx.compose.ui.Alignment.CenterVertically,
                horizontalArrangement = Arrangement.SpaceBetween
            ) {
                Text(
                    text = "Quick Access",
                    style = MaterialTheme.typography.titleMedium,
                    color = MaterialTheme.colorScheme.primary
                )
                androidx.compose.material3.IconButton(onClick = onToggleAssistSettings) {
                    Icon(Icons.Default.Settings, contentDescription = "Configure Assist")
                }
            }

            // Assist Settings Panel (Collapsible)
            androidx.compose.animation.AnimatedVisibility(visible = assistSettingsExpanded) {
                ElevatedCard(modifier = Modifier.fillMaxWidth()) {
                    Column(
                        modifier = Modifier.padding(16.dp),
                        verticalArrangement = Arrangement.spacedBy(12.dp)
                    ) {
                        Text("Assist Settings", style = MaterialTheme.typography.titleSmall)
                        Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                            OutlinedTextField(
                                value = suggestionLookbackDays.toString(),
                                onValueChange = onSuggestionLookbackDaysChange,
                                label = { Text("Days") },
                                modifier = Modifier.weight(1f),
                                keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Number)
                            )
                            OutlinedTextField(
                                value = suggestionTopN.toString(),
                                onValueChange = onSuggestionTopNChange,
                                label = { Text("Top N") },
                                modifier = Modifier.weight(1f),
                                keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Number)
                            )
                        }
                        
                         OutlinedTextField(
                            value = quickActivitySearch,
                            onValueChange = { quickActivitySearch = it },
                            label = { Text("Search & Add Quick Activity") },
                            modifier = Modifier.fillMaxWidth()
                        )
                        
                        // Available candidates logic (simplified for brevity, keeping core logic)
                        val searchToken = quickActivitySearch.trim()
                        if (searchToken.isNotEmpty()) {
                             val candidates = availableActivityNames.filter { 
                                 !quickActivities.contains(it) && it.contains(searchToken, true) 
                             }.take(5)
                             com.example.tracer.ui.components.SimpleFlowRow(horizontalGap = 8.dp, verticalGap = 8.dp) {
                                 candidates.forEach { candidate ->
                                     androidx.compose.material3.InputChip(
                                         selected = false,
                                         onClick = { onQuickActivitiesUpdate((quickActivities + candidate).distinct()) },
                                         label = { Text("+ $candidate") }
                                     )
                                 }
                             }
                        }
                    }
                }
            }

            // Quick Activities List (Always Visible)
            if (quickActivities.isNotEmpty()) {
                com.example.tracer.ui.components.SimpleFlowRow(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalGap = 8.dp,
                    verticalGap = 8.dp
                ) {
                    quickActivities.forEach { activity ->
                        val isSelected = recordContent.trim() == activity
                        val isDeleting = assistSettingsExpanded // Delete mode enabled when settings open
                        
                        if (isDeleting) {
                             androidx.compose.material3.InputChip(
                                selected = isSelected,
                                onClick = { 
                                    onQuickActivitiesUpdate(quickActivities.filter { it != activity }) 
                                },
                                label = { Text("$activity X") },
                                colors = androidx.compose.material3.InputChipDefaults.inputChipColors(
                                    containerColor = MaterialTheme.colorScheme.errorContainer,
                                    labelColor = MaterialTheme.colorScheme.onErrorContainer
                                )
                            )
                        } else {
                            // Quick Action Button style
                            androidx.compose.material3.SuggestionChip(
                                onClick = { onRecordContentChange(activity) },
                                label = { 
                                    Text(
                                        text = activity, 
                                        style = MaterialTheme.typography.bodyMedium,
                                        modifier = Modifier.padding(vertical = 4.dp),
                                        maxLines = 1,
                                        overflow = androidx.compose.ui.text.style.TextOverflow.Ellipsis
                                    ) 
                                },
                                colors = androidx.compose.material3.SuggestionChipDefaults.suggestionChipColors(
                                    containerColor = if (isSelected) MaterialTheme.colorScheme.primaryContainer else MaterialTheme.colorScheme.surfaceContainerHigh,
                                    labelColor = if (isSelected) MaterialTheme.colorScheme.onPrimaryContainer else MaterialTheme.colorScheme.onSurface
                                ),
                                border = androidx.compose.material3.SuggestionChipDefaults.suggestionChipBorder(
                                    enabled = true,
                                    borderColor = if (isSelected) MaterialTheme.colorScheme.primary else MaterialTheme.colorScheme.outline
                                )
                            )
                        }
                    }
                }
            } else {
                 Text("No quick activities. Click Settings to add.", style = MaterialTheme.typography.bodySmall)
            }

            // Suggestions Section (Collapsible)
            Column(modifier = Modifier.fillMaxWidth()) {
                androidx.compose.material3.TextButton(
                    onClick = onToggleSuggestions,
                    modifier = Modifier.fillMaxWidth()
                ) {
                    Text(if (suggestionsVisible) "Hide Suggestions" else "Show Suggestions")
                    Spacer(Modifier.width(8.dp))
                    Icon(
                        if (suggestionsVisible) Icons.Default.KeyboardArrowUp else Icons.Default.KeyboardArrowDown,
                        contentDescription = null
                    )
                }
                
                androidx.compose.animation.AnimatedVisibility(visible = suggestionsVisible) {
                    Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
                        if (isSuggestionsLoading) {
                            Text("Loading...", style = MaterialTheme.typography.bodySmall)
                        } else if (suggestedActivities.isEmpty()) {
                            Text("No suggestions.", style = MaterialTheme.typography.bodySmall)
                        } else {
                             com.example.tracer.ui.components.SimpleFlowRow(
                                modifier = Modifier.fillMaxWidth(),
                                horizontalGap = 8.dp,
                                verticalGap = 8.dp
                            ) {
                                suggestedActivities.forEach { activity ->
                                    androidx.compose.material3.SuggestionChip(
                                        onClick = { onSuggestedActivityClick(activity) },
                                        label = {
                                            Text(
                                                text = activity,
                                                maxLines = 1,
                                                overflow = androidx.compose.ui.text.style.TextOverflow.Ellipsis
                                            )
                                        }
                                    )
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

private fun formatCurrentTime(): String = DISPLAY_TIME_FORMATTER.format(Date())
