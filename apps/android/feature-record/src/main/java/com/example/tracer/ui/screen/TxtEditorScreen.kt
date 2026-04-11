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
import androidx.compose.material3.Button
import androidx.compose.material3.ElevatedCard
import androidx.compose.material3.FloatingActionButton
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.produceState
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.record.R
import kotlinx.coroutines.launch

internal enum class TxtOutputMode {
    ALL,
    DAY
}

@Composable
fun TxtEditorSection(
    txtStorageGateway: TxtStorageGateway,
    inspectionEntries: List<TxtInspectionEntry>,
    availableMonths: List<String>,
    selectedMonth: String,
    logicalDayTarget: RecordLogicalDayTarget,
    onOpenPreviousMonth: () -> Unit,
    onOpenNextMonth: () -> Unit,
    onOpenMonth: (String) -> Unit,
    selectedHistoryFile: String,
    onRefreshHistory: () -> Unit,
    editableHistoryContent: String,
    onEditableHistoryContentChange: (String) -> Unit,
    onSaveHistoryFile: () -> Unit,
    inlineStatusText: String,
    onCreateCurrentMonthTxt: () -> Unit
) {
    var outputMode by remember { mutableStateOf(TxtOutputMode.DAY) }
    var dayMarkerInput by remember {
        mutableStateOf("0101")
    }
    var autoDayMarkerLoadedKey by remember { mutableStateOf("") }
    var isEditorContentVisible by remember(selectedHistoryFile) { mutableStateOf(false) }
    val coroutineScope = rememberCoroutineScope()
    val parsedAvailableMonths = remember(inspectionEntries) {
        inspectionEntries
            .mapNotNull { it.headerMonth }
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

    LaunchedEffect(selectedHistoryFile, selectedMonth) {
        outputMode = if (selectedMonth.isBlank()) {
            TxtOutputMode.ALL
        } else {
            TxtOutputMode.DAY
        }
    }

    LaunchedEffect(selectedHistoryFile, selectedMonth, logicalDayTarget) {
        if (selectedHistoryFile.isBlank()) {
            autoDayMarkerLoadedKey = ""
            return@LaunchedEffect
        }
        val loadKey = "$selectedHistoryFile@$selectedMonth@$logicalDayTarget"
        if (autoDayMarkerLoadedKey == loadKey) {
            return@LaunchedEffect
        }

        val markerResult = txtStorageGateway.defaultTxtDayMarker(
            selectedMonth = selectedMonth,
            targetDateIso = resolveLogicalDayTargetDate(logicalDayTarget).toString()
        )
        dayMarkerInput = markerResult.normalizedDayMarker.ifBlank { dayMarkerInput }
        autoDayMarkerLoadedKey = loadKey
    }

    // Auto-load TXT list when entering the tab to avoid requiring manual refresh.
    LaunchedEffect(selectedHistoryFile, inspectionEntries) {
        if (selectedHistoryFile.isBlank() && inspectionEntries.isEmpty()) {
            onRefreshHistory()
        }
    }

    val normalizedDayMarkerInput = remember(dayMarkerInput) {
        dayMarkerInput.filter { it.isDigit() }.take(4)
    }
    // DAY mode is runtime-driven on purpose: the screen owns only UI state,
    // while shared month-TXT day-block semantics now live in core.
    val dayBlockEditorState by produceState(
        initialValue = TxtDayBlockResolveResult(
            ok = false,
            normalizedDayMarker = normalizedDayMarkerInput,
            found = false,
            isMarkerValid = false,
            canSave = false,
            dayBody = "",
            dayContentIsoDate = null,
            message = ""
        ),
        editableHistoryContent,
        normalizedDayMarkerInput,
        selectedMonth
    ) {
        value = txtStorageGateway.resolveTxtDayBlock(
            content = editableHistoryContent,
            dayMarker = normalizedDayMarkerInput,
            selectedMonth = selectedMonth
        )
    }
    val canSaveCurrentContent = outputMode == TxtOutputMode.ALL ||
        (dayBlockEditorState.ok && dayBlockEditorState.canSave)
    val showSaveFab = selectedHistoryFile.isNotEmpty() && isEditorContentVisible

    // Empty-state: no TXT files exist yet (typical for fresh release installs).
    // Show a guidance card so users can bootstrap their first month TXT file.
    val showEmptyState = inspectionEntries.isEmpty()

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
                if (showEmptyState) {
                    TxtEmptyStateCard(onCreateCurrentMonthTxt = onCreateCurrentMonthTxt)
                } else if (selectedHistoryFile.isNotEmpty()) {
                    TxtEditorContentCard(
                        selectedHistoryFile = selectedHistoryFile,
                        selectedMonth = selectedMonth,
                        outputMode = outputMode,
                        onOutputModeChange = { nextMode ->
                            if (nextMode == TxtOutputMode.DAY && outputMode != TxtOutputMode.DAY) {
                                coroutineScope.launch {
                                    val markerResult = txtStorageGateway.defaultTxtDayMarker(
                                        selectedMonth = selectedMonth,
                                        targetDateIso = resolveLogicalDayTargetDate(logicalDayTarget).toString()
                                    )
                                    dayMarkerInput =
                                        markerResult.normalizedDayMarker.ifBlank { dayMarkerInput }
                                }
                            }
                            outputMode = nextMode
                        },
                        dayBlockEditorState = dayBlockEditorState,
                        dayMarkerInput = dayMarkerInput,
                        onDayMarkerInputChange = { value ->
                            dayMarkerInput = value.filter { it.isDigit() }.take(4)
                        },
                        onDayEditorBodyChange = { value ->
                            coroutineScope.launch {
                                val replaced = txtStorageGateway.replaceTxtDayBlock(
                                    content = editableHistoryContent,
                                    dayMarker = normalizedDayMarkerInput,
                                    editedDayBody = value
                                )
                                if (replaced.ok) {
                                    onEditableHistoryContentChange(replaced.updatedContent)
                                }
                            }
                        },
                        inlineStatusText = inlineStatusText,
                        isEditorContentVisible = isEditorContentVisible,
                        onToggleEditorContentVisibility = {
                            isEditorContentVisible = !isEditorContentVisible
                        },
                        editableHistoryContent = editableHistoryContent,
                        onEditableHistoryContentChange = onEditableHistoryContentChange
                    )
                } else {
                    TxtSelectionHintCard()
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

@Composable
private fun TxtSelectionHintCard() {
    ElevatedCard(
        modifier = Modifier
            .fillMaxWidth()
            .padding(horizontal = 8.dp)
    ) {
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .padding(16.dp),
            verticalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            Text(
                text = stringResource(R.string.txt_unselected_state_title),
                style = MaterialTheme.typography.titleMedium,
                color = MaterialTheme.colorScheme.primary
            )
            Text(
                text = stringResource(R.string.txt_unselected_state_description),
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
        }
    }
}

// Empty-state guidance card for users who have no TXT files yet.
// This is the primary entry point for fresh release installs where no
// bundled test data exists. Creating the current month TXT bootstraps
// the file with mandatory header lines (yYYYY, mMM) so that the
// Record Input flow can immediately append day blocks on demand.
@Composable
private fun TxtEmptyStateCard(onCreateCurrentMonthTxt: () -> Unit) {
    ElevatedCard(
        modifier = Modifier
            .fillMaxWidth()
            .padding(horizontal = 8.dp)
    ) {
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .padding(16.dp),
            verticalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            Text(
                text = stringResource(R.string.txt_empty_state_title),
                style = MaterialTheme.typography.titleMedium,
                color = MaterialTheme.colorScheme.primary
            )
            Text(
                text = stringResource(R.string.txt_empty_state_description),
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
            Button(
                onClick = onCreateCurrentMonthTxt,
                modifier = Modifier.fillMaxWidth()
            ) {
                Text(stringResource(R.string.txt_action_create_current_month))
            }
        }
    }
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
