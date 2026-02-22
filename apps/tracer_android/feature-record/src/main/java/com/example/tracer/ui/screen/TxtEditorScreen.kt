package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.ArrowForward
import androidx.compose.material3.FloatingActionButton
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.record.R
import java.util.Calendar
import java.util.Locale

internal enum class TxtOutputMode {
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
    selectedHistoryFile: String,
    onRefreshHistory: () -> Unit,
    editableHistoryContent: String,
    onEditableHistoryContentChange: (String) -> Unit,
    onSaveHistoryFile: () -> Unit,
    inlineStatusText: String
) {
    var outputMode by remember { mutableStateOf(TxtOutputMode.DAY) }
    var dayMarkerInput by remember {
        mutableStateOf("0101")
    }
    var autoDayMarkerLoadedKey by remember { mutableStateOf("") }
    var isEditorContentVisible by remember(selectedHistoryFile) { mutableStateOf(false) }
    val parsedAvailableMonths = remember(availableMonths) {
        availableMonths
            .mapNotNull(::parseYearMonthKey)
            .distinctBy { it.key }
            .sortedBy { it.key }
    }
    val monthsByYear = remember(parsedAvailableMonths) {
        parsedAvailableMonths
            .groupBy { it.year }
            .mapValues { (_, value) -> value.map { it.month }.distinct().sorted() }
    }
    val availableYears = remember(monthsByYear) {
        monthsByYear.keys.sorted()
    }
    val selectedYear = parseYearMonthKey(selectedMonth)?.year
        ?: availableYears.lastOrNull().orEmpty()
    val availableMonthValues = remember(monthsByYear, selectedYear) {
        if (selectedYear.isBlank()) {
            emptyList()
        } else {
            monthsByYear[selectedYear].orEmpty()
        }
    }
    val selectedMonthValue = parseYearMonthKey(selectedMonth)?.month
        ?.takeIf { availableMonthValues.contains(it) }
        ?: availableMonthValues.lastOrNull().orEmpty()

    LaunchedEffect(selectedMonth) {
        outputMode = TxtOutputMode.DAY
    }

    LaunchedEffect(selectedHistoryFile, selectedMonth) {
        if (selectedHistoryFile.isBlank()) {
            autoDayMarkerLoadedKey = ""
            return@LaunchedEffect
        }
        val loadKey = "$selectedHistoryFile@$selectedMonth"
        if (autoDayMarkerLoadedKey == loadKey) {
            return@LaunchedEffect
        }

        dayMarkerInput = defaultDayMarkerForSelectedMonth(selectedMonth)
        autoDayMarkerLoadedKey = loadKey
    }

    // Auto-load TXT list when entering the tab to avoid requiring manual refresh.
    LaunchedEffect(selectedHistoryFile, availableMonths) {
        if (selectedHistoryFile.isBlank() && availableMonths.isEmpty()) {
            onRefreshHistory()
        }
    }

    val normalizedDayMarker = dayMarkerInput.filter { it.isDigit() }.take(4)
    val dayEditorStateForSave = remember(editableHistoryContent, normalizedDayMarker) {
        buildDayBlockEditorState(
            content = editableHistoryContent,
            dayMarker = normalizedDayMarker
        )
    }
    val canSaveCurrentContent = outputMode == TxtOutputMode.ALL ||
        (dayEditorStateForSave.isMarkerValid && dayEditorStateForSave.found)
    val showSaveFab = selectedHistoryFile.isNotEmpty() && isEditorContentVisible

    Box(modifier = Modifier.fillMaxSize()) {
        Column(modifier = Modifier.fillMaxSize()) {
            TxtMonthNavigationCard(
                selectedMonth = selectedMonth,
                onOpenPreviousMonth = onOpenPreviousMonth,
                onOpenNextMonth = onOpenNextMonth,
                onOpenMonth = onOpenMonth,
                availableYears = availableYears,
                selectedYear = selectedYear,
                selectedMonthValue = selectedMonthValue,
                monthsByYear = monthsByYear,
                onRefreshHistory = onRefreshHistory
            )

            Column(
                modifier = Modifier
                    .fillMaxWidth()
                    .weight(1f)
                    .padding(top = 16.dp)
                    .padding(bottom = if (showSaveFab) 24.dp else 0.dp)
                    .verticalScroll(rememberScrollState()),
                verticalArrangement = Arrangement.spacedBy(16.dp)
            ) {
                if (selectedHistoryFile.isNotEmpty()) {
                    TxtEditorContentCard(
                        selectedHistoryFile = selectedHistoryFile,
                        selectedMonth = selectedMonth,
                        outputMode = outputMode,
                        onOutputModeChange = { nextMode ->
                            if (nextMode == TxtOutputMode.DAY && outputMode != TxtOutputMode.DAY) {
                                dayMarkerInput = defaultDayMarkerForSelectedMonth(selectedMonth)
                            }
                            outputMode = nextMode
                        },
                        dayMarkerInput = dayMarkerInput,
                        onDayMarkerInputChange = { value ->
                            dayMarkerInput = value.filter { it.isDigit() }.take(4)
                        },
                        inlineStatusText = inlineStatusText,
                        isEditorContentVisible = isEditorContentVisible,
                        onToggleEditorContentVisibility = {
                            isEditorContentVisible = !isEditorContentVisible
                        },
                        editableHistoryContent = editableHistoryContent,
                        onEditableHistoryContentChange = onEditableHistoryContentChange
                    )
                }
            }
        }

        if (showSaveFab) {
            val canSave = canSaveCurrentContent
            FloatingActionButton(
                onClick = {
                    if (canSave) {
                        onSaveHistoryFile()
                    }
                },
                containerColor = if (canSave) {
                    MaterialTheme.colorScheme.primaryContainer
                } else {
                    MaterialTheme.colorScheme.surfaceVariant
                },
                contentColor = if (canSave) {
                    MaterialTheme.colorScheme.onPrimaryContainer
                } else {
                    MaterialTheme.colorScheme.onSurfaceVariant
                },
                modifier = Modifier
                    .align(Alignment.BottomEnd)
                    .padding(end = 16.dp, bottom = 12.dp)
            ) {
                Icon(
                    imageVector = Icons.AutoMirrored.Filled.ArrowForward,
                    contentDescription = stringResource(R.string.txt_cd_ingest)
                )
            }
        }
    }
}

internal data class DayBlockEditorState(
    val text: String,
    val found: Boolean,
    val isMarkerValid: Boolean
)

internal fun buildDayBlockEditorState(content: String, dayMarker: String): DayBlockEditorState {
    if (!isValidDayMarker(dayMarker)) {
        return DayBlockEditorState(
            text = "",
            found = false,
            isMarkerValid = false
        )
    }

    val lines = content.lines().map { it.trimEnd('\r') }
    val startIndex = lines.indexOfFirst { it.trim() == dayMarker }
    if (startIndex < 0) {
        return DayBlockEditorState(
            text = "",
            found = false,
            isMarkerValid = true
        )
    }

    val endIndex = findDayBlockEndIndex(lines, startIndex)
    val bodyLines = lines.subList(startIndex + 1, endIndex)
    return DayBlockEditorState(
        text = bodyLines.joinToString(separator = "\n"),
        found = true,
        isMarkerValid = true
    )
}

internal fun mergeDayBlockContent(
    fullContent: String,
    dayMarker: String,
    editedDayBody: String
): String {
    if (!isValidDayMarker(dayMarker)) {
        return fullContent
    }

    val lines = fullContent.lines().map { it.trimEnd('\r') }.toMutableList()
    val startIndex = lines.indexOfFirst { it.trim() == dayMarker }
    if (startIndex < 0) {
        return fullContent
    }

    val endIndex = findDayBlockEndIndex(lines, startIndex)
    val normalizedDayBodyLines = normalizeEditedDayBody(dayMarker, editedDayBody)
    lines.subList(startIndex + 1, endIndex).clear()
    lines.addAll(startIndex + 1, normalizedDayBodyLines)

    return lines.joinToString(separator = "\n")
}

private fun normalizeEditedDayBody(dayMarker: String, editedDayBody: String): List<String> {
    val lines = editedDayBody.lines().map { it.trimEnd('\r') }.toMutableList()

    // Keep trailing blank lines so Enter in DAY editor is reflected immediately.
    // Backward-compat: tolerate pasted content that still includes MMDD header.
    if (lines.firstOrNull()?.trim() == dayMarker) {
        lines.removeAt(0)
    }
    return lines
}

private fun findDayBlockEndIndex(lines: List<String>, startIndex: Int): Int {
    for (index in (startIndex + 1) until lines.size) {
        if (isDayMarkerLine(lines[index])) {
            return index
        }
    }
    return lines.size
}

private fun defaultDayMarkerForSelectedMonth(selectedMonth: String): String {
    val now = Calendar.getInstance()
    val fallbackMonth = now.get(Calendar.MONTH) + 1
    val fallbackDay = now.get(Calendar.DAY_OF_MONTH)
    val parsed = parseYearMonthKey(selectedMonth)
    if (parsed == null) {
        return String.format(Locale.US, "%02d%02d", fallbackMonth, fallbackDay)
    }

    val selectedYear = parsed.year.toIntOrNull()
    val selectedMonth = parsed.month.toIntOrNull()
    if (selectedYear == null || selectedMonth == null) {
        return String.format(Locale.US, "%02d%02d", fallbackMonth, fallbackDay)
    }

    val maxDay = Calendar.getInstance().run {
        clear()
        set(Calendar.YEAR, selectedYear)
        set(Calendar.MONTH, selectedMonth - 1)
        getActualMaximum(Calendar.DAY_OF_MONTH)
    }
    val targetDay = fallbackDay.coerceAtMost(maxDay)
    return String.format(Locale.US, "%02d%02d", selectedMonth, targetDay)
}

private fun isDayMarkerLine(line: String): Boolean {
    return isValidDayMarker(line.trim())
}

private fun isValidDayMarker(value: String): Boolean {
    if (value.length != 4 || !value.all { it.isDigit() }) {
        return false
    }
    val month = value.substring(0, 2).toIntOrNull() ?: return false
    val day = value.substring(2, 4).toIntOrNull() ?: return false
    return month in 1..12 && day in 1..31
}

private data class YearMonthKey(
    val year: String,
    val month: String
) {
    val key: String
        get() = "$year-$month"
}

private fun parseYearMonthKey(value: String): YearMonthKey? {
    val normalized = value.trim()
    val match = Regex("""^(\d{4})-(\d{2})$""").matchEntire(normalized)
        ?: return null
    val year = match.groupValues[1]
    val month = match.groupValues[2]
    val monthInt = month.toIntOrNull() ?: return null
    if (monthInt !in 1..12) {
        return null
    }
    return YearMonthKey(year = year, month = month)
}
