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
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.produceState
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.dp
import android.util.Log
import com.example.tracer.feature.record.R
import com.example.tracer.ui.components.CalendarAvailability
import java.time.Clock
import java.time.LocalDate
import java.time.format.DateTimeFormatter
import java.time.format.DateTimeParseException
import kotlinx.coroutines.launch

enum class TxtOutputMode {
    ALL,
    DAY
}

private const val TXT_TAB_LOG_TAG = "TxtTab"

@Composable
fun TxtEditorSection(
    txtStorageGateway: TxtStorageGateway,
    inspectionEntries: List<TxtInspectionEntry>,
    availableMonths: List<String>,
    selectedMonth: String,
    logicalDayTarget: RecordLogicalDayTarget,
    txtHistoryLoaded: Boolean = false,
    initialDayMarker: String = "",
    logicalDayClock: Clock,
    onOpenPreviousMonth: () -> Unit,
    onOpenNextMonth: () -> Unit,
    onOpenMonth: (String) -> Unit,
    selectedHistoryFile: String,
    selectedHistoryContent: String,
    onRefreshHistory: () -> Unit,
    editableHistoryContent: String,
    onEditableHistoryContentChange: (String) -> Unit,
    onDayMarkerPersist: (String) -> Unit = {},
    onDiscardUnsavedHistoryDraft: () -> Unit,
    onSaveHistoryFile: () -> Unit,
    onSaveHistoryRepresentationOnly: suspend (String) -> TxtFileContentResult,
    initialOutputMode: TxtOutputMode = TxtOutputMode.DAY,
    onOutputModePersist: (TxtOutputMode) -> Unit = {},
    bottomContentPadding: Dp = 0.dp,
    inlineStatusText: String,
    onCreateCurrentMonthTxt: () -> Unit
) {
    LaunchedEffect(Unit) {
        Log.d(
            TXT_TAB_LOG_TAG,
            "compose enter selectedFile=$selectedHistoryFile selectedMonth=$selectedMonth " +
                "inspectionCount=${inspectionEntries.size} historyLoaded=$txtHistoryLoaded " +
                "outputMode=$initialOutputMode initialMarker=$initialDayMarker"
        )
    }
    val sessionController = remember(selectedHistoryFile, selectedMonth) {
        val normalizedInitialDayMarker = initialDayMarker.filter { it.isDigit() }.take(4)
        TxtEditorSessionController(
            initialState = TxtEditorSessionState(
                outputMode = initialOutputMode,
                // Keep the first frame empty when no marker has been restored yet. The runtime
                // loads the logical-day marker asynchronously; using 0101 here makes the UI
                // visibly jump from Jan 1 to the resolved target date on every tab re-entry.
                dayMarkerInput = normalizedInitialDayMarker,
                autoDayMarkerLoadedKey = if (normalizedInitialDayMarker.isBlank()) {
                    ""
                } else {
                    "$selectedHistoryFile@$selectedMonth@$logicalDayTarget"
                }
            )
        )
    }
    val runtimeCoordinator = remember(txtStorageGateway, logicalDayClock) {
        TxtEditorRuntimeCoordinator(
            txtStorageGateway = txtStorageGateway,
            logicalDayClock = logicalDayClock
        )
    }
    val sessionState = sessionController.state
    val coroutineScope = rememberCoroutineScope()
    fun updateDayMarkerInput(value: String) {
        sessionController.updateDayMarkerInput(value)
        Log.d(TXT_TAB_LOG_TAG, "day marker input changed value=${sessionController.state.dayMarkerInput}")
        onDayMarkerPersist(sessionController.state.dayMarkerInput)
    }
    var activityNameConversionStatus by remember(selectedHistoryFile, selectedMonth) {
        mutableStateOf("")
    }
    val parsedAvailableMonths = remember(inspectionEntries) {
        inspectionEntries
            .mapNotNull { it.headerMonth }
            .mapNotNull(::parseYearMonthKey)
            .distinctBy { it.key }
            .sortedBy { it.key }
    }
    val calendarAvailability = remember(parsedAvailableMonths) {
        // CalendarAvailability accepts strict YYYY-MM keys. Do not use
        // YearMonthKey.toString() here: the data class's default rendering is
        // "YearMonthKey(year=..., month=...)", which makes every month invalid
        // and disables the TXT month picker.
        CalendarAvailability.fromMonthKeys(parsedAvailableMonths.map { it.key })
    }
    val monthsByYear = calendarAvailability.monthsByYear
    val availableYears = calendarAvailability.years
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

    LaunchedEffect(selectedHistoryFile, selectedMonth, initialOutputMode) {
        sessionController.syncSelectionContext(
            selectedHistoryFile = selectedHistoryFile,
            selectedMonth = selectedMonth
        )
        sessionController.updateOutputMode(initialOutputMode)
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
        Log.d(
            TXT_TAB_LOG_TAG,
            "auto marker load start selectedFile=$selectedHistoryFile selectedMonth=$selectedMonth " +
                "logicalDayTarget=$logicalDayTarget currentMarker=${sessionController.state.dayMarkerInput} " +
                "loadedKey=${sessionController.state.autoDayMarkerLoadedKey}"
        )
        runtimeCoordinator.syncAutoDayMarkerIfNeeded(
            selectedHistoryFile = selectedHistoryFile,
            selectedMonth = selectedMonth,
            logicalDayTarget = logicalDayTarget,
            sessionController = sessionController
        )
        if (selectedHistoryFile.isNotBlank()) {
            onDayMarkerPersist(sessionController.state.dayMarkerInput)
        }
        Log.d(
            TXT_TAB_LOG_TAG,
            "auto marker load complete selectedFile=$selectedHistoryFile selectedMonth=$selectedMonth " +
                "marker=${sessionController.state.dayMarkerInput} loadedKey=${sessionController.state.autoDayMarkerLoadedKey}"
        )
    }

    // Auto-load TXT list when entering the tab to avoid requiring manual refresh.
    LaunchedEffect(selectedHistoryFile, inspectionEntries) {
        if (selectedHistoryFile.isBlank() && inspectionEntries.isEmpty()) {
            Log.d(TXT_TAB_LOG_TAG, "history load requested from empty initial state")
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
        initialValue = TxtEditableDayBlockResult(
            resolveResult = TxtDayBlockResolveResult(
                ok = false,
                normalizedDayMarker = normalizedDayMarkerInput,
                found = false,
                isMarkerValid = false,
                canSave = false,
                dayBody = "",
                dayContentIsoDate = null,
                message = ""
            ),
            monthContent = sessionController.currentMonthContent(editableHistoryContent),
            canEdit = false
        ),
        sessionState.allDraftState,
        normalizedDayMarkerInput,
        selectedMonth
    ) {
        value = runtimeCoordinator.prepareEditableDayBlock(
            monthContent = sessionController.currentMonthContent(editableHistoryContent),
            dayMarker = normalizedDayMarkerInput,
            selectedMonth = selectedMonth
        )
    }
    val resolvedDayBlockState = dayBlockEditorState.resolveResult
    LaunchedEffect(
        sessionState.outputMode,
        resolvedDayBlockState.dayBody,
        resolvedDayBlockState.normalizedDayMarker,
        resolvedDayBlockState.dayContentIsoDate,
        dayBlockEditorState.monthContent
    ) {
        if (dayBlockEditorState.monthContent != sessionController.currentMonthContent(editableHistoryContent)) {
            onEditableHistoryContentChange(dayBlockEditorState.monthContent)
            sessionController.syncExternalMonthDraft(
                selectedHistoryContent = selectedHistoryContent,
                editableHistoryContent = dayBlockEditorState.monthContent
            )
        }
        if (sessionState.outputMode == TxtOutputMode.DAY) {
            sessionController.syncResolvedDayBody(resolvedDayBlockState.dayBody)
        }
    }
    val currentDay = remember(
        selectedMonth,
        normalizedDayMarkerInput,
        resolvedDayBlockState.dayContentIsoDate
    ) {
        resolveDisplayedCurrentDay(
            selectedMonth = selectedMonth,
            normalizedDayMarker = normalizedDayMarkerInput,
            resolvedIsoDate = resolvedDayBlockState.dayContentIsoDate
        )
    }
    val filteredInlineStatusText = remember(inlineStatusText) {
        if (inlineStatusText.startsWith("open month ->")) {
            ""
        } else {
            inlineStatusText
        }
    }

    val canEditDay = dayBlockEditorState.canEdit
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
    // Do not show the final empty state while the cold-start history inspection is still in
    // flight. Otherwise the first frame briefly renders "No TXT files yet" before the editor
    // appears for an existing file.
    val showEmptyState = txtHistoryLoaded && inspectionEntries.isEmpty()
    LaunchedEffect(txtHistoryLoaded, inspectionEntries.size, selectedHistoryFile, sessionState.outputMode, sessionState.dayMarkerInput) {
        Log.d(
            TXT_TAB_LOG_TAG,
            "render state historyLoaded=$txtHistoryLoaded inspectionCount=${inspectionEntries.size} " +
                "selectedFile=$selectedHistoryFile selectedMonth=$selectedMonth showEmpty=$showEmptyState " +
                "outputMode=${sessionState.outputMode} marker=${sessionState.dayMarkerInput} " +
                "markerReady=${sessionState.autoDayMarkerLoadedKey.isNotBlank()}"
        )
    }

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
                        onDayMarkerInputChange = ::updateDayMarkerInput,
                        onOpenMonth = onOpenMonth
                    )
                },
                onOpenNextDay = {
                    navigateToAdjacentDay(
                        currentDay = currentDay,
                        dayOffset = 1,
                        selectedMonth = selectedMonth,
                        onPendingDayChange = sessionController::updatePendingOpenedDay,
                        onDayMarkerInputChange = ::updateDayMarkerInput,
                        onOpenMonth = onOpenMonth
                    )
                },
                onOpenDay = { day ->
                    navigateToDay(
                        targetDay = day,
                        selectedMonth = selectedMonth,
                        onPendingDayChange = sessionController::updatePendingOpenedDay,
                        onDayMarkerInputChange = ::updateDayMarkerInput,
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
                    .verticalScroll(rememberScrollState())
                    .padding(bottom = bottomContentPadding),
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
                            Log.d(
                                TXT_TAB_LOG_TAG,
                                "output mode changed from=${sessionState.outputMode} to=$nextMode"
                            )
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
                            onOutputModePersist(nextMode)
                        },
                        activityNameTargetMode = sessionState.activityNameTargetMode,
                        onActivityNameTargetModeChange = { targetMode ->
                            sessionController.updateActivityNameTargetMode(targetMode)
                            if (sessionState.outputMode == TxtOutputMode.ALL) {
                                coroutineScope.launch {
                                    val conversion = runtimeCoordinator.convertActivityNames(
                                        content = sessionController.currentMonthContent(
                                            editableHistoryContent
                                        ),
                                        targetMode = targetMode
                                    )
                                    if (!sessionController.isCurrentSelection(
                                            selectedHistoryFile = selectedHistoryFile,
                                            selectedMonth = selectedMonth
                                        )
                                    ) {
                                        return@launch
                                    }
                                    if (conversion.ok) {
                                        val saveResult = onSaveHistoryRepresentationOnly(
                                            conversion.convertedContent
                                        )
                                        if (saveResult.ok) {
                                            val persistedContent = saveResult.content.ifBlank {
                                                conversion.convertedContent
                                            }
                                            sessionController.updateAllDraft(persistedContent)
                                            sessionController.syncExternalMonthDraft(
                                                selectedHistoryContent = persistedContent,
                                                editableHistoryContent = persistedContent
                                            )
                                            activityNameConversionStatus = ""
                                        } else {
                                            activityNameConversionStatus = saveResult.message
                                        }
                                    } else {
                                        activityNameConversionStatus = conversion.message
                                    }
                                }
                            }
                        },
                        dayBlockEditorState = resolvedDayBlockState,
                        dayMarkerInput = sessionState.dayMarkerInput,
                        onDayMarkerInputChange = ::updateDayMarkerInput,
                        onOpenDay = { day ->
                            navigateToDay(
                                targetDay = day,
                                selectedMonth = selectedMonth,
                                onPendingDayChange = sessionController::updatePendingOpenedDay,
                                onDayMarkerInputChange = ::updateDayMarkerInput,
                                onOpenMonth = onOpenMonth
                            )
                        },
                        inlineStatusText = activityNameConversionStatus.ifBlank {
                            filteredInlineStatusText
                        },
                        isEditorContentVisible = sessionState.isEditorContentVisible,
                        onToggleEditorContentVisibility = {
                            if (sessionState.isEditorContentVisible) {
                                sessionController.closeEditorSession(
                                    resolvedDayBody = resolvedDayBlockState.dayBody,
                                    onDiscardAllDraft = onDiscardUnsavedHistoryDraft
                                )
                            } else {
                                if (sessionState.outputMode == TxtOutputMode.DAY) {
                                    coroutineScope.launch {
                                        runtimeCoordinator.openDayEditor(
                                            sessionController = sessionController,
                                            selectedHistoryFile = selectedHistoryFile,
                                            selectedMonth = selectedMonth,
                                            logicalDayTarget = logicalDayTarget,
                                            fallbackMonthContent = editableHistoryContent,
                                            persistedMonthContent = selectedHistoryContent,
                                            currentResolveResult = resolvedDayBlockState,
                                            onMonthContentReconciled = { reconciledMonthContent ->
                                                onEditableHistoryContentChange(reconciledMonthContent)
                                                sessionController.syncExternalMonthDraft(
                                                    selectedHistoryContent = selectedHistoryContent,
                                                    editableHistoryContent = reconciledMonthContent
                                                )
                                            }
                                        )
                                    }
                                } else {
                                    // Opening the editor should hydrate DAY from the current
                                    // resolved body so a previously abandoned draft does not leak
                                    // into the next editing session.
                                    sessionController.openEditor(resolvedDayBlockState.dayBody)
                                }
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
                        },
                        dayMarkerReady = sessionState.autoDayMarkerLoadedKey.isNotBlank() ||
                            normalizedDayMarkerInput.length == 4
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
