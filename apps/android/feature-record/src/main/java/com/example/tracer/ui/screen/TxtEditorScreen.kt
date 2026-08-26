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
import com.example.tracer.ui.components.LocalFullscreenOverlayHost
import java.time.Clock
import java.time.LocalDate
import java.time.format.DateTimeFormatter
import java.time.format.DateTimeParseException
import java.security.MessageDigest
import kotlinx.coroutines.launch


enum class TxtOutputMode {
    ALL,
    DAY
}
private const val TXT_TAB_LOG_TAG = "TxtTab"

@Composable
fun TxtEditorSection(
    txtStorageGateway: TxtStorageGateway,
    canonicalCatalogRoots: List<CanonicalPathNode> = emptyList(),
    isCanonicalCatalogLoading: Boolean = false,
    canonicalCatalogStatusText: String = "",
    onCanonicalCatalogRequested: () -> Unit = {},
    collapsedCanonicalRootPaths: Set<String> = emptySet(),
    orderedCanonicalRootPaths: List<String> = emptyList(),
    onCollapsedCanonicalRootPathsChange: (Set<String>) -> Unit = {},
    onOrderedCanonicalRootPathsChange: (List<String>) -> Unit = {},
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
    onSaveHistoryFile: () -> Unit,
    onSaveHistoryRepresentationOnly: suspend (String) -> TxtFileContentResult,
    bottomContentPadding: Dp = 0.dp,
    embeddedInScrollableParent: Boolean = false,
    inlineStatusText: String,
    onCreateCurrentMonthTxt: () -> Unit
) {
    LaunchedEffect(Unit) {
        Log.d(
            TXT_TAB_LOG_TAG,
            "compose enter selectedFile=$selectedHistoryFile selectedMonth=$selectedMonth " +
                "inspectionCount=${inspectionEntries.size} historyLoaded=$txtHistoryLoaded " +
                "outputMode=DAY initialMarker=$initialDayMarker"
        )
    }
    val sessionController = remember(selectedHistoryFile, selectedMonth) {
        val normalizedInitialDayMarker = initialDayMarkerForSelectedMonth(
            initialDayMarker = initialDayMarker,
            selectedMonth = selectedMonth
        )
        TxtEditorSessionController(
            initialState = TxtEditorSessionState(
                outputMode = TxtOutputMode.DAY,
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

    LaunchedEffect(selectedHistoryFile, selectedMonth) {
        Log.d(
            TXT_TAB_LOG_TAG,
            "selection sync start file=$selectedHistoryFile month=$selectedMonth " +
                "mode=DAY source=${selectedHistoryContent.txtDebugSignature()} " +
                "editable=${editableHistoryContent.txtDebugSignature()}"
        )
        sessionController.syncSelectionContext(
            selectedHistoryFile = selectedHistoryFile,
            selectedMonth = selectedMonth
        )
        sessionController.updateOutputMode(TxtOutputMode.DAY)
        Log.d(
            TXT_TAB_LOG_TAG,
            "selection sync complete key=${sessionController.state.selectionContextKey} " +
                "allDraft=${sessionController.state.allDraftState.draftText.txtDebugSignature()} " +
                "dayDraft=${sessionController.state.dayDraftState.draftText.txtDebugSignature()}"
        )
    }
    val rawEditorOverlayHost = LocalFullscreenOverlayHost.current

    LaunchedEffect(selectedHistoryFile, selectedHistoryContent, editableHistoryContent) {
        if (selectedHistoryFile.isNotBlank()) {
            Log.d(
                TXT_TAB_LOG_TAG,
                "external draft sync start file=$selectedHistoryFile source=${selectedHistoryContent.txtDebugSignature()} " +
                    "editable=${editableHistoryContent.txtDebugSignature()}"
            )
            sessionController.syncExternalMonthDraft(
                selectedHistoryContent = selectedHistoryContent,
                editableHistoryContent = editableHistoryContent
            )
            Log.d(
                TXT_TAB_LOG_TAG,
                "external draft sync complete file=$selectedHistoryFile " +
                    "allBaseline=${sessionController.state.allDraftState.baselineText.txtDebugSignature()} " +
                    "allDraft=${sessionController.state.allDraftState.draftText.txtDebugSignature()}"
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
    val structuredDayEdit by produceState<TxtDayEditResolveResult?>(
        initialValue = null,
        sessionState.allDraftState,
        normalizedDayMarkerInput,
        selectedMonth,
        dayBlockEditorState.canEdit,
        dayBlockEditorState.monthContent
    ) {
        value = if (dayBlockEditorState.canEdit) {
            runtimeCoordinator.resolveDayEdit(
                monthContent = dayBlockEditorState.monthContent,
                dayMarker = normalizedDayMarkerInput,
                selectedMonth = selectedMonth
            )
        } else {
            null
        }
    }
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
    LaunchedEffect(
        txtHistoryLoaded,
        inspectionEntries.size,
        selectedHistoryFile,
        selectedMonth,
        sessionState.outputMode,
        sessionState.dayMarkerInput,
        dayBlockEditorState.monthContent,
        resolvedDayBlockState.dayBody,
        editorUiState.editorText,
        structuredDayEdit
    ) {
        Log.d(
            TXT_TAB_LOG_TAG,
            "render state historyLoaded=$txtHistoryLoaded inspectionCount=${inspectionEntries.size} " +
                "selectedFile=$selectedHistoryFile selectedMonth=$selectedMonth showEmpty=$showEmptyState " +
                "outputMode=${sessionState.outputMode} marker=${sessionState.dayMarkerInput} " +
                "markerReady=${sessionState.autoDayMarkerLoadedKey.isNotBlank()} " +
                "month=${dayBlockEditorState.monthContent.txtDebugSignature()} " +
                "dayResolve=(ok=${resolvedDayBlockState.ok},found=${resolvedDayBlockState.found}," +
                "canSave=${resolvedDayBlockState.canSave},body=${resolvedDayBlockState.dayBody.txtDebugSignature()}," +
                "message=${resolvedDayBlockState.message.take(120)}) " +
                "editor=${editorUiState.editorText.txtDebugSignature()} " +
                "structured=(ok=${structuredDayEdit?.ok},found=${structuredDayEdit?.found}," +
                "events=${structuredDayEdit?.events?.size}," +
                "remark=${structuredDayEdit?.dayRemark?.txtDebugSignature().orEmpty()}," +
                "message=${structuredDayEdit?.message?.take(120).orEmpty()})"
        )
    }

    Box(
        modifier = if (embeddedInScrollableParent) {
            Modifier.fillMaxWidth()
        } else {
            Modifier.fillMaxSize()
        }
    ) {
        Column(
            modifier = if (embeddedInScrollableParent) {
                Modifier.fillMaxWidth()
            } else {
                Modifier.fillMaxSize()
            }
        ) {
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
                monthsByYear = monthsByYear
            )

            Column(
                modifier = Modifier
                    .fillMaxWidth()
                    .then(
                        if (embeddedInScrollableParent) {
                            Modifier.padding(top = 16.dp)
                        } else {
                            Modifier
                                .weight(1f)
                                .padding(top = 16.dp)
                                .verticalScroll(rememberScrollState())
                                .padding(bottom = bottomContentPadding)
                        }
                    ),
                verticalArrangement = Arrangement.spacedBy(16.dp)
            ) {
                if (showEmptyState) {
                    TxtEmptyStateCard(onCreateCurrentMonthTxt = onCreateCurrentMonthTxt)
                } else if (selectedHistoryFile.isNotEmpty()) {
                    TxtEditorContentCard(
                        selectedHistoryFile = selectedHistoryFile,
                        currentDay = currentDay,
                        onConvertActivityNames = { targetMode ->
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
                        },
                        dayBlockEditorState = resolvedDayBlockState,
                        inlineStatusText = activityNameConversionStatus.ifBlank { filteredInlineStatusText },
                        onCanonicalCatalogRequested = onCanonicalCatalogRequested,
                        onReloadTxtData = onRefreshHistory,
                        onOpenRawEditor = { rawOutputMode ->
                            sessionController.updateOutputMode(rawOutputMode)
                            sessionController.openEditor(resolvedDayBlockState.dayBody)
                            requireNotNull(rawEditorOverlayHost).show {
                                TxtRawEditorRoute(
                                    overlayHost = requireNotNull(rawEditorOverlayHost),
                                    sessionController = sessionController,
                                    runtimeCoordinator = runtimeCoordinator,
                                    selectedMonth = selectedMonth,
                                    selectedDay = currentDay,
                                    canEditDay = canEditDay,
                                    dayMarker = normalizedDayMarkerInput,
                                    resolvedDayBody = resolvedDayBlockState.dayBody,
                                    onMergedMonthContent = onEditableHistoryContentChange,
                                    onSaveHistoryFile = onSaveHistoryFile
                                )
                            }
                        },
                        structuredDayEdit = structuredDayEdit,
                        canonicalCatalogRoots = canonicalCatalogRoots,
                        isCanonicalCatalogLoading = isCanonicalCatalogLoading,
                        canonicalCatalogStatusText = canonicalCatalogStatusText,
                        collapsedCanonicalRootPaths = collapsedCanonicalRootPaths,
                        orderedCanonicalRootPaths = orderedCanonicalRootPaths,
                        onCollapsedCanonicalRootPathsChange = onCollapsedCanonicalRootPathsChange,
                        onOrderedCanonicalRootPathsChange = onOrderedCanonicalRootPathsChange,
                        onStructuredDayEditApply = { dayRemark, events ->
                            coroutineScope.launch {
                                runtimeCoordinator.applyDayEdit(
                                    monthContent = sessionController.currentMonthContent(
                                        editableHistoryContent
                                    ),
                                    dayMarker = normalizedDayMarkerInput,
                                    selectedMonth = selectedMonth,
                                    dayRemark = dayRemark,
                                    events = events,
                                    onMergedMonthContent = onEditableHistoryContentChange,
                                    onSaveHistoryFile = onSaveHistoryFile
                                )
                            }
                        },
                        onStructuredDayActivityReplace = {
                                sourceActivityToken,
                                targetActivityToken,
                                dayRemark,
                                events ->
                            runtimeCoordinator.replaceDayActivityToken(
                                monthContent = sessionController.currentMonthContent(
                                    editableHistoryContent
                                ),
                                dayMarker = normalizedDayMarkerInput,
                                selectedMonth = selectedMonth,
                                dayRemark = dayRemark,
                                events = events,
                                sourceActivityToken = sourceActivityToken,
                                targetActivityToken = targetActivityToken,
                                onMergedMonthContent = onEditableHistoryContentChange,
                                onSaveHistoryFile = onSaveHistoryFile
                            )
                        },
                        onPrepareMonthActivityEdits = {
                            runtimeCoordinator.prepareMonthActivityEdits(
                                monthContent = sessionController.currentMonthContent(
                                    editableHistoryContent
                                ),
                                selectedMonth = selectedMonth
                            )
                        },
                        onReplaceMonthActivity = {
                                snapshot,
                                sourceActivityToken,
                                targetActivityToken ->
                            runtimeCoordinator.replaceMonthActivityToken(
                                snapshot = snapshot,
                                sourceActivityToken = sourceActivityToken,
                                targetActivityToken = targetActivityToken,
                                onMergedMonthContent = onEditableHistoryContentChange,
                                onSaveHistoryFile = onSaveHistoryFile
                            )
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
private fun TxtRawEditorRoute(
    overlayHost: com.example.tracer.ui.components.FullscreenOverlayHost,
    sessionController: TxtEditorSessionController,
    runtimeCoordinator: TxtEditorRuntimeCoordinator,
    selectedMonth: String,
    selectedDay: LocalDate?,
    canEditDay: Boolean,
    dayMarker: String,
    resolvedDayBody: String,
    onMergedMonthContent: (String) -> Unit,
    onSaveHistoryFile: () -> Unit
) {
    val outputMode = sessionController.state.outputMode
    val editorUiState = sessionController.deriveEditorUiState(canEditDay)
    val coroutineScope = rememberCoroutineScope()
    TxtRawEditorFullScreen(
        outputMode = outputMode,
        selectedMonth = selectedMonth,
        selectedDay = selectedDay,
        value = editorUiState.editorText,
        hasUnsavedChanges = editorUiState.hasUnsavedChanges,
        canSave = editorUiState.canIngest,
        readOnly = outputMode == TxtOutputMode.DAY && !canEditDay,
        onOutputModeChange = sessionController::updateOutputMode,
        onValueChange = sessionController::onEditorTextChange,
        onSave = {
            coroutineScope.launch {
                if (runtimeCoordinator.ingestCurrentEditor(
                        sessionController = sessionController,
                        canEditDay = canEditDay,
                        dayMarker = dayMarker,
                        onMergedMonthContent = onMergedMonthContent,
                        onSaveHistoryFile = onSaveHistoryFile
                    )
                ) {
                    sessionController.updateOutputMode(TxtOutputMode.DAY)
                    overlayHost.dismiss()
                }
            }
        },
        onDiscard = {
            sessionController.closeEditorSession(
                resolvedDayBody = resolvedDayBody,
                onDiscardAllDraft = {
                    onMergedMonthContent(sessionController.state.allDraftState.baselineText)
                }
            )
            sessionController.updateOutputMode(TxtOutputMode.DAY)
            overlayHost.dismiss()
        }
    )
}

/** Diagnostic-only content identifier: enough to trace state propagation without logging TXT. */
private fun String.txtDebugSignature(): String {
    val digest = MessageDigest.getInstance("SHA-256")
        .digest(toByteArray())
        .take(6)
        .joinToString(separator = "") { byte -> "%02x".format(byte.toInt() and 0xff) }
    return "len=$length,sha256=$digest"
}

/**
 * The persisted marker is only a MMDD value. Do not reuse it for another
 * month: doing so marks a stale day as already loaded and prevents the
 * runtime from selecting the selected month's default day.
 */
internal fun initialDayMarkerForSelectedMonth(
    initialDayMarker: String,
    selectedMonth: String
): String {
    val normalizedMarker = initialDayMarker.filter { it.isDigit() }.take(4)
    val selectedMonthDigits = selectedMonth.takeLast(2)
    return normalizedMarker.takeIf {
        it.length == 4 && selectedMonthDigits.length == 2 && it.take(2) == selectedMonthDigits
    }.orEmpty()
}
