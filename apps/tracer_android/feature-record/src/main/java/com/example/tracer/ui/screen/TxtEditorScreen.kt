package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.ArrowBack
import androidx.compose.material.icons.automirrored.filled.ArrowForward
import androidx.compose.material.icons.filled.Add
import androidx.compose.material.icons.filled.ArrowDropDown
import androidx.compose.material.icons.filled.Check
import androidx.compose.material.icons.filled.Refresh
import androidx.compose.material3.Button
import androidx.compose.material3.DropdownMenu
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.ElevatedCard
import androidx.compose.material3.Icon
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
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import java.text.SimpleDateFormat
import java.util.Calendar
import java.util.Date
import java.util.Locale

private enum class TxtOutputMode {
    ALL,
    DAY
}

@Composable
fun TxtEditorSection(
    availableMonths: List<String>,
    selectedMonth: String,
    onOpenPreviousMonth: () -> Unit,
    onOpenNextMonth: () -> Unit,
    onOpenMonth: (String) -> Unit,
    onCreateCurrentMonthTxt: () -> Unit,
    selectedHistoryFile: String,
    onRefreshHistory: () -> Unit,
    editableHistoryContent: String,
    onEditableHistoryContentChange: (String) -> Unit,
    onSaveHistoryFile: () -> Unit,
    inlineStatusText: String
) {
    val cal = remember { Calendar.getInstance() }
    var monthMenuExpanded by remember { mutableStateOf(false) }
    var outputMode by remember { mutableStateOf(TxtOutputMode.ALL) }
    var dayMarkerInput by remember {
        mutableStateOf(SimpleDateFormat("MMdd", Locale.US).format(cal.time))
    }

    var pickedYearText by remember { mutableStateOf(cal.get(Calendar.YEAR).toString()) }
    var pickedMonth by remember { mutableStateOf(cal.get(Calendar.MONTH) + 1) }

    LaunchedEffect(selectedMonth) {
        val parsed = parseMonthKey(selectedMonth)
        if (parsed != null) {
            pickedYearText = parsed.first.toString()
            pickedMonth = parsed.second
        }
        outputMode = TxtOutputMode.ALL
    }

    val monthItems = remember {
        (1..12).map { month ->
            month to String.format(Locale.US, "%02d", month)
        }
    }
    val pickedYear = pickedYearText.toIntOrNull()
    val isYearValid = pickedYear != null && pickedYear in 1..9999

    Column(
        modifier = Modifier.fillMaxWidth(),
        verticalArrangement = Arrangement.spacedBy(16.dp)
    ) {
        // Navigation & Creation Card
        ElevatedCard(modifier = Modifier.fillMaxWidth()) {
            Column(
                modifier = Modifier.padding(16.dp),
                verticalArrangement = Arrangement.spacedBy(12.dp)
            ) {
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.SpaceBetween,
                    verticalAlignment = androidx.compose.ui.Alignment.CenterVertically
                ) {
                    androidx.compose.material3.IconButton(onClick = onOpenPreviousMonth) {
                        Icon(Icons.AutoMirrored.Filled.ArrowBack, contentDescription = "Previous Month")
                    }

                    Box {
                        androidx.compose.material3.TextButton(onClick = { monthMenuExpanded = true }) {
                            Text(
                                text = selectedMonth.ifEmpty { "Select File" },
                                style = MaterialTheme.typography.titleLarge,
                                color = MaterialTheme.colorScheme.primary
                            )
                            Spacer(modifier = Modifier.width(4.dp))
                            Icon(Icons.Filled.ArrowDropDown, contentDescription = null)
                        }

                        DropdownMenu(
                            expanded = monthMenuExpanded,
                            onDismissRequest = { monthMenuExpanded = false }
                        ) {
                            if (availableMonths.isEmpty()) {
                                DropdownMenuItem(
                                    text = { Text("No files") },
                                    onClick = { monthMenuExpanded = false }
                                )
                            } else {
                                availableMonths.forEach { month ->
                                    DropdownMenuItem(
                                        text = { Text(month) },
                                        onClick = {
                                            monthMenuExpanded = false
                                            onOpenMonth(month)
                                        }
                                    )
                                }
                            }
                        }
                    }

                    androidx.compose.material3.IconButton(onClick = onOpenNextMonth) {
                        Icon(Icons.AutoMirrored.Filled.ArrowForward, contentDescription = "Next Month")
                    }
                }

                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.Center
                ) {
                    androidx.compose.material3.TextButton(onClick = onCreateCurrentMonthTxt) {
                        Icon(Icons.Filled.Add, contentDescription = null)
                        Spacer(modifier = Modifier.width(4.dp))
                        Text("New Current")
                    }
                    androidx.compose.material3.TextButton(onClick = onRefreshHistory) {
                        Icon(Icons.Filled.Refresh, contentDescription = null)
                        Spacer(modifier = Modifier.width(4.dp))
                        Text("Refresh List")
                    }
                }
            }
        }

        if (selectedHistoryFile.isNotEmpty()) {
            val normalizedDayMarker = dayMarkerInput.filter { it.isDigit() }.take(4)
            val editorText = if (outputMode == TxtOutputMode.ALL) {
                editableHistoryContent
            } else {
                extractDayBlockOutput(editableHistoryContent, normalizedDayMarker)
            }
            val dayModeHint = if (outputMode == TxtOutputMode.DAY) {
                "Day output is preview-only. Switch to ALL to edit and save."
            } else {
                ""
            }

            // Editor Card
            ElevatedCard(modifier = Modifier.fillMaxWidth()) {
                Column(
                    modifier = Modifier.padding(16.dp),
                    verticalArrangement = Arrangement.spacedBy(12.dp)
                ) {
                    Text(
                        text = "Editor: $selectedHistoryFile",
                        style = MaterialTheme.typography.titleMedium,
                        color = MaterialTheme.colorScheme.primary
                    )

                    Row(
                        modifier = Modifier.fillMaxWidth(),
                        horizontalArrangement = Arrangement.spacedBy(8.dp)
                    ) {
                        Button(
                            onClick = { outputMode = TxtOutputMode.ALL },
                            enabled = outputMode != TxtOutputMode.ALL,
                            modifier = Modifier.weight(1f)
                        ) {
                            Text("ALL")
                        }
                        OutlinedButton(
                            onClick = { outputMode = TxtOutputMode.DAY },
                            enabled = outputMode != TxtOutputMode.DAY,
                            modifier = Modifier.weight(1f)
                        ) {
                            Text("DAY")
                        }
                    }
                    if (outputMode == TxtOutputMode.DAY) {
                        OutlinedTextField(
                            value = dayMarkerInput,
                            onValueChange = { value ->
                                dayMarkerInput = value.filter { it.isDigit() }.take(4)
                            },
                            label = { Text("Target Day (MMDD)") },
                            singleLine = true,
                            modifier = Modifier.fillMaxWidth()
                        )
                    }
                    
                    if (dayModeHint.isNotBlank()) {
                         Text(
                             text = dayModeHint,
                             style = MaterialTheme.typography.bodySmall,
                             color = MaterialTheme.colorScheme.tertiary
                         )
                    }

                    if (inlineStatusText.isNotBlank()) {
                        val isError = inlineStatusText.contains("fail", ignoreCase = true) ||
                            inlineStatusText.contains("error", ignoreCase = true)
                        val statusColor = if (isError) {
                            MaterialTheme.colorScheme.error
                        } else {
                            MaterialTheme.colorScheme.primary
                        }
                        Text(
                            text = inlineStatusText,
                            style = MaterialTheme.typography.bodySmall,
                            color = statusColor,
                            modifier = Modifier.fillMaxWidth()
                        )
                    }

                    OutlinedTextField(
                        value = editorText,
                        onValueChange = onEditableHistoryContentChange,
                        readOnly = outputMode == TxtOutputMode.DAY,
                        label = {
                            Text(if (outputMode == TxtOutputMode.ALL) "Content" else "Preview")
                        },
                        modifier = Modifier.fillMaxWidth(),
                        minLines = 8,
                        textStyle = MaterialTheme.typography.bodyMedium.copy(fontFamily = androidx.compose.ui.text.font.FontFamily.Monospace)
                    )
                    
                    Button(
                        onClick = onSaveHistoryFile,
                        enabled = outputMode == TxtOutputMode.ALL,
                        modifier = Modifier.fillMaxWidth()
                    ) {
                        Icon(Icons.Filled.Check, contentDescription = null)
                        Spacer(modifier = Modifier.width(8.dp))
                        Text("Save & Sync")
                    }
                }
            }
        }
    }
}

private fun parseMonthKey(monthKey: String): Pair<Int, Int>? {
    val match = Regex("""^(\d{4})-(\d{2})$""").matchEntire(monthKey.trim())
        ?: return null
    val year = match.groupValues[1].toIntOrNull() ?: return null
    val month = match.groupValues[2].toIntOrNull() ?: return null
    if (month !in 1..12) {
        return null
    }
    return year to month
}

private fun extractDayBlockOutput(content: String, dayMarker: String): String {
    if (!isValidDayMarker(dayMarker)) {
        return "Invalid day marker. Use MMDD (example: 0214)."
    }

    val lines = content.lines()
    val startIndex = lines.indexOfFirst { it.trim() == dayMarker }
    if (startIndex < 0) {
        return "Day $dayMarker not found in current TXT."
    }

    var endIndex = lines.size
    for (index in (startIndex + 1) until lines.size) {
        if (isDayMarkerLine(lines[index])) {
            endIndex = index
            break
        }
    }

    return lines.subList(startIndex, endIndex).joinToString(separator = "\n")
}

private fun isDayMarkerLine(line: String): Boolean {
    val trimmed = line.trim()
    return trimmed.length == 4 && trimmed.all { it.isDigit() }
}

private fun isValidDayMarker(value: String): Boolean {
    if (value.length != 4 || !value.all { it.isDigit() }) {
        return false
    }
    val month = value.substring(0, 2).toIntOrNull() ?: return false
    val day = value.substring(2, 4).toIntOrNull() ?: return false
    return month in 1..12 && day in 1..31
}
