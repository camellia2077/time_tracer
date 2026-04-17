package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.Button
import androidx.compose.material3.ElevatedCard
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.produceState
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.record.R
import java.time.LocalDate
import java.time.format.DateTimeFormatter
import java.time.format.DateTimeParseException
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
    selectedHistoryContent: String,
    onRefreshHistory: () -> Unit,
    editableHistoryContent: String,
    onEditableHistoryContentChange: (String) -> Unit,
    onDiscardUnsavedHistoryDraft: () -> Unit,
    onSaveHistoryFile: () -> Unit,
    inlineStatusText: String,
    onCreateCurrentMonthTxt: () -> Unit
) {
    val sessionController = remember(selectedHistoryFile) { TxtEditorSessionController() }
    val runtimeCoordinator = remember(txtStorageGateway) { TxtEditorRuntimeCoordinator(txtStorageGateway) }
    val sessionState = sessionController.state
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
        sessionController.syncSelectionContext(
            selectedHistoryFile = selectedHistoryFile,
            selectedMonth = selectedMonth
        )
    }

    LaunchedEffect(selectedHistoryFile, selectedHistoryContent, editableHistoryContent) {
        if (selectedHistoryFile.isNotBlank()) {
            sessionController.syncExternalMonthDraft(
                selectedHistoryContent = selectedHistoryContent,
                editableHistoryContent = editableHistoryContent
            )
        }
    }

    LaunchedEffect(selectedHistoryFile, selectedMonth, logicalDayTarget) {
        runtimeCoordinator.syncAutoDayMarkerIfNeeded(
            selectedHistoryFile = selectedHistoryFile,
            selectedMonth = selectedMonth,
            logicalDayTarget = logicalDayTarget,
            sessionController = sessionController
        )
    }

    // Auto-load TXT list when entering the tab to avoid requiring manual refresh.
    LaunchedEffect(selectedHistoryFile, inspectionEntries) {
        if (selectedHistoryFile.isBlank() && inspectionEntries.isEmpty()) {
            onRefreshHistory()
        }
    }

    val normalizedDayMarkerInput = remember(sessionState.dayMarkerInput) {
        sessionController.normalizedDayMarkerInput
    }
    // Shared runtime still owns month-TXT day-block semantics, but the editor session now owns
    // both DAY and ALL drafts. Resolving a day block from the session's month content keeps
    // ALL -> DAY mode switches inside one coherent editing session instead of snapping back to
    // the last ViewModel-backed month string on every mode change.
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
        sessionState.allDraftState,
        normalizedDayMarkerInput,
        selectedMonth
    ) {
        value = runtimeCoordinator.resolveDayBlock(
            monthContent = sessionController.currentMonthContent(editableHistoryContent),
            dayMarker = normalizedDayMarkerInput,
            selectedMonth = selectedMonth
        )
    }
    LaunchedEffect(
        sessionState.outputMode,
        dayBlockEditorState.dayBody,
        dayBlockEditorState.normalizedDayMarker,
        dayBlockEditorState.dayContentIsoDate
    ) {
        if (sessionState.outputMode == TxtOutputMode.DAY) {
            sessionController.syncResolvedDayBody(dayBlockEditorState.dayBody)
        }
    }
    val currentDay = remember(
        selectedMonth,
        normalizedDayMarkerInput,
        dayBlockEditorState.dayContentIsoDate
    ) {
        resolveDisplayedCurrentDay(
            selectedMonth = selectedMonth,
            normalizedDayMarker = normalizedDayMarkerInput,
            resolvedIsoDate = dayBlockEditorState.dayContentIsoDate
        )
    }
    val filteredInlineStatusText = remember(inlineStatusText) {
        if (inlineStatusText.startsWith("open month ->")) {
            ""
        } else {
            inlineStatusText
        }
    }

    val canEditDay = dayBlockEditorState.ok && dayBlockEditorState.canSave
    val editorUiState = remember(
        sessionState.outputMode,
        sessionState.allDraftState,
        sessionState.dayDraftState,
        canEditDay
    ) {
        sessionController.deriveEditorUiState(canEditDay = canEditDay)
    }

    // Empty-state: no TXT files exist yet (typical for fresh release installs).
    // Show a guidance card so users can bootstrap their first month TXT file.
    val showEmptyState = inspectionEntries.isEmpty()

    Box(modifier = Modifier.fillMaxSize()) {
        Column(modifier = Modifier.fillMaxSize()) {
            TxtMonthNavigationCard(
                selectedMonth = selectedMonth,
                currentDay = currentDay,
                onOpenPreviousMonth = onOpenPreviousMonth,
                onOpenNextMonth = onOpenNextMonth,
                onOpenPreviousDay = {
                    navigateToAdjacentDay(
                        currentDay = currentDay,
                        dayOffset = -1,
                        selectedMonth = selectedMonth,
                        onPendingDayChange = sessionController::updatePendingOpenedDay,
                        onDayMarkerInputChange = sessionController::updateDayMarkerInput,
                        onOpenMonth = onOpenMonth
                    )
                },
                onOpenNextDay = {
                    navigateToAdjacentDay(
                        currentDay = currentDay,
                        dayOffset = 1,
                        selectedMonth = selectedMonth,
                        onPendingDayChange = sessionController::updatePendingOpenedDay,
                        onDayMarkerInputChange = sessionController::updateDayMarkerInput,
                        onOpenMonth = onOpenMonth
                    )
                },
                onOpenDay = { day ->
                    navigateToDay(
                        targetDay = day,
                        selectedMonth = selectedMonth,
                        onPendingDayChange = sessionController::updatePendingOpenedDay,
                        onDayMarkerInputChange = sessionController::updateDayMarkerInput,
                        onOpenMonth = onOpenMonth
                    )
                },
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
                    .verticalScroll(rememberScrollState()),
                verticalArrangement = Arrangement.spacedBy(16.dp)
            ) {
                if (showEmptyState) {
                    TxtEmptyStateCard(onCreateCurrentMonthTxt = onCreateCurrentMonthTxt)
                } else if (selectedHistoryFile.isNotEmpty()) {
                    TxtEditorContentCard(
                        selectedHistoryFile = selectedHistoryFile,
                        selectedMonth = selectedMonth,
                        currentDay = currentDay,
                        outputMode = sessionState.outputMode,
                        onOutputModeChange = { nextMode ->
                            if (nextMode == TxtOutputMode.DAY && sessionState.outputMode != TxtOutputMode.DAY) {
                                coroutineScope.launch {
                                    val normalizedDayMarker = runtimeCoordinator.loadDefaultDayMarker(
                                        selectedMonth = selectedMonth,
                                        logicalDayTarget = logicalDayTarget
                                    )
                                    sessionController.applyAutoDayMarker(
                                        selectedHistoryFile = selectedHistoryFile,
                                        selectedMonth = selectedMonth,
                                        logicalDayTarget = logicalDayTarget,
                                        normalizedDayMarker = normalizedDayMarker
                                    )
                                }
                            }
                            sessionController.updateOutputMode(nextMode)
                        },
                        dayBlockEditorState = dayBlockEditorState,
                        dayMarkerInput = sessionState.dayMarkerInput,
                        onDayMarkerInputChange = sessionController::updateDayMarkerInput,
                        inlineStatusText = filteredInlineStatusText,
                        isEditorContentVisible = sessionState.isEditorContentVisible,
                        onToggleEditorContentVisibility = {
                            if (sessionState.isEditorContentVisible) {
                                sessionController.closeEditorSession(
                                    resolvedDayBody = dayBlockEditorState.dayBody,
                                    onDiscardAllDraft = onDiscardUnsavedHistoryDraft
                                )
                            } else {
                                // Opening the editor should hydrate DAY from the current
                                // resolved body so a previously abandoned draft does not leak
                                // into the next editing session.
                                sessionController.openEditor(dayBlockEditorState.dayBody)
                            }
                        },
                        editorText = editorUiState.editorText,
                        hasUnsavedChanges = editorUiState.hasUnsavedChanges,
                        canEditDay = canEditDay,
                        canIngest = editorUiState.canIngest,
                        onEditorTextChange = { nextValue ->
                            sessionController.onEditorTextChange(nextValue)
                        },
                        onIngest = {
                            coroutineScope.launch {
                                runtimeCoordinator.ingestCurrentEditor(
                                    sessionController = sessionController,
                                    canEditDay = canEditDay,
                                    dayMarker = normalizedDayMarkerInput,
                                    onMergedMonthContent = onEditableHistoryContentChange,
                                    onSaveHistoryFile = onSaveHistoryFile
                                )
                            }
                        }
                    )
                } else {
                    TxtSelectionHintCard()
                }
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

private fun resolveDisplayedCurrentDay(
    selectedMonth: String,
    normalizedDayMarker: String,
    resolvedIsoDate: String?
): LocalDate? {
    parseIsoDateOrNull(resolvedIsoDate)?.let { return it }
    val yearMonth = parseYearMonthKey(selectedMonth) ?: return null
    if (normalizedDayMarker.length != 4) {
        return null
    }
    val month = normalizedDayMarker.take(2).toIntOrNull() ?: return null
    val day = normalizedDayMarker.drop(2).toIntOrNull() ?: return null
    return try {
        LocalDate.of(yearMonth.year.toInt(), month, day)
    } catch (_: RuntimeException) {
        null
    }
}

private fun parseIsoDateOrNull(value: String?): LocalDate? {
    if (value.isNullOrBlank()) {
        return null
    }
    return try {
        LocalDate.parse(value)
    } catch (_: DateTimeParseException) {
        null
    }
}

private fun navigateToAdjacentDay(
    currentDay: LocalDate?,
    dayOffset: Long,
    selectedMonth: String,
    onPendingDayChange: (LocalDate?) -> Unit,
    onDayMarkerInputChange: (String) -> Unit,
    onOpenMonth: (String) -> Unit
) {
    val baseDay = currentDay ?: return
    navigateToDay(
        targetDay = baseDay.plusDays(dayOffset),
        selectedMonth = selectedMonth,
        onPendingDayChange = onPendingDayChange,
        onDayMarkerInputChange = onDayMarkerInputChange,
        onOpenMonth = onOpenMonth
    )
}

internal fun navigateToDay(
    targetDay: LocalDate,
    selectedMonth: String,
    onPendingDayChange: (LocalDate?) -> Unit,
    onDayMarkerInputChange: (String) -> Unit,
    onOpenMonth: (String) -> Unit
) {
    onDayMarkerInputChange(formatDayMarker(targetDay))
    val targetMonth = formatMonthKey(targetDay)
    if (targetMonth == selectedMonth) {
        onPendingDayChange(null)
        return
    }
    onPendingDayChange(targetDay)
    onOpenMonth(targetMonth)
}

internal fun formatMonthKey(date: LocalDate): String =
    date.format(DateTimeFormatter.ofPattern("yyyy-MM"))

internal fun formatDayMarker(date: LocalDate): String =
    date.format(DateTimeFormatter.ofPattern("MMdd"))
