package com.example.tracer

import java.time.Clock

internal class TxtEditorRuntimeCoordinator(
    private val txtStorageGateway: TxtStorageGateway,
    private val logicalDayClock: Clock
) {
    // TXT editor runtime must reuse the same logical-day clock as Record. Otherwise Record and
    // TXT can disagree about which calendar date "yesterday/today" points at around the 06:00
    // cutoff, especially in tests or CI where the host default zone may differ from the target
    // device zone. The injected clock keeps preview/day-marker behavior aligned with Record.
    suspend fun syncAutoDayMarkerIfNeeded(
        sessionController: TxtEditorSessionController,
        selectedHistoryFile: String,
        selectedMonth: String,
        logicalDayTarget: RecordLogicalDayTarget
    ) {
        if (selectedHistoryFile.isBlank()) {
            return
        }
        if (sessionController.tryApplyPendingOpenedDay(selectedHistoryFile, selectedMonth)) {
            return
        }
        val loadKey = sessionController.defaultAutoDayMarkerLoadKey(
            selectedHistoryFile = selectedHistoryFile,
            selectedMonth = selectedMonth,
            logicalDayTarget = logicalDayTarget
        )
        if (sessionController.hasLoadedAutoDayMarker(loadKey)) {
            return
        }

        val markerResult = txtStorageGateway.defaultTxtDayMarker(
            selectedMonth = selectedMonth,
            targetDateIso = resolveLogicalDayTargetDate(
                logicalDayTarget = logicalDayTarget,
                clock = logicalDayClock
            ).toString()
        )
        sessionController.applyAutoDayMarker(
            selectedHistoryFile = selectedHistoryFile,
            selectedMonth = selectedMonth,
            logicalDayTarget = logicalDayTarget,
            normalizedDayMarker = markerResult.normalizedDayMarker
        )
    }

    suspend fun loadDefaultDayMarker(
        selectedMonth: String,
        logicalDayTarget: RecordLogicalDayTarget
    ): String = txtStorageGateway.defaultTxtDayMarker(
        selectedMonth = selectedMonth,
        targetDateIso = resolveLogicalDayTargetDate(
            logicalDayTarget = logicalDayTarget,
            clock = logicalDayClock
        ).toString()
    ).normalizedDayMarker

    suspend fun resolveDayBlock(
        monthContent: String,
        dayMarker: String,
        selectedMonth: String
    ): TxtDayBlockResolveResult = txtStorageGateway.resolveTxtDayBlock(
        content = monthContent,
        dayMarker = dayMarker,
        selectedMonth = selectedMonth
    )

    suspend fun resolveDayEdit(
        monthContent: String,
        dayMarker: String,
        selectedMonth: String
    ): TxtDayEditResolveResult = txtStorageGateway.resolveTxtDayEdit(
        content = monthContent,
        dayMarker = dayMarker,
        selectedMonth = selectedMonth
    )

    suspend fun applyDayEdit(
        monthContent: String,
        dayMarker: String,
        selectedMonth: String,
        dayRemark: String,
        events: List<TxtDayEditEvent>,
        onMergedMonthContent: (String) -> Unit,
        onSaveHistoryFile: () -> Unit
    ): Boolean {
        val applied = txtStorageGateway.applyTxtDayEdit(
            content = monthContent,
            dayMarker = dayMarker,
            selectedMonth = selectedMonth,
            dayRemark = dayRemark,
            events = events
        )
        if (!applied.ok) {
            return false
        }
        onMergedMonthContent(applied.updatedContent)
        onSaveHistoryFile()
        return true
    }

    suspend fun convertActivityNames(
        content: String,
        targetMode: TxtActivityNameTargetMode
    ): TxtActivityNameConversionResult = txtStorageGateway.convertTxtActivityNames(
        content = content,
        direction = when (targetMode) {
            TxtActivityNameTargetMode.ALIAS ->
                TxtActivityNameMappingDirection.CANONICAL_TO_ALIAS
            TxtActivityNameTargetMode.CANONICAL ->
                TxtActivityNameMappingDirection.ALIAS_TO_CANONICAL
        }
    )

    suspend fun prepareEditableDayBlock(
        monthContent: String,
        dayMarker: String,
        selectedMonth: String
    ): TxtEditableDayBlockResult {
        if (!hasCanonicalMonthHeaders(monthContent)) {
            return TxtEditableDayBlockResult(
                resolveResult = TxtDayBlockResolveResult(
                    ok = false,
                    normalizedDayMarker = dayMarker.filter { it.isDigit() }.take(4),
                    found = false,
                    isMarkerValid = false,
                    canSave = false,
                    dayBody = "",
                    dayContentIsoDate = null,
                    message = ""
                ),
                monthContent = monthContent,
                canEdit = false
            )
        }
        val resolved = resolveDayBlock(
            monthContent = monthContent,
            dayMarker = dayMarker,
            selectedMonth = selectedMonth
        )
        if (!resolved.ok || resolved.found || !resolved.isMarkerValid) {
            return TxtEditableDayBlockResult(
                resolveResult = resolved,
                monthContent = monthContent,
                canEdit = resolved.ok && resolved.canSave
            )
        }
        if (resolved.dayContentIsoDate == null) {
            return TxtEditableDayBlockResult(
                resolveResult = resolved,
                monthContent = monthContent,
                canEdit = false
            )
        }

        val monthWithEmptyDay = appendEmptyDayBlock(
            monthContent = monthContent,
            dayMarker = resolved.normalizedDayMarker.ifBlank { dayMarker }
        )
        val seededResolve = resolved.copy(
            found = true,
            canSave = true,
            dayBody = ""
        )
        return TxtEditableDayBlockResult(
            resolveResult = seededResolve,
            monthContent = monthWithEmptyDay,
            canEdit = true
        )
    }

    suspend fun ingestCurrentEditor(
        sessionController: TxtEditorSessionController,
        canEditDay: Boolean,
        dayMarker: String,
        onMergedMonthContent: (String) -> Unit,
        onSaveHistoryFile: () -> Unit
    ): Boolean {
        val sessionState = sessionController.state
        val editorUiState = sessionController.deriveEditorUiState(canEditDay = canEditDay)
        if (!editorUiState.canIngest) {
            return false
        }
        return if (sessionState.outputMode == TxtOutputMode.ALL) {
            onMergedMonthContent(sessionState.allDraftState.draftText)
            onSaveHistoryFile()
            sessionController.state = TxtEditorSessionReducer.hideEditor(
                TxtEditorSessionReducer.markAllDraftPersisted(
                    sessionState,
                    sessionState.allDraftState.draftText
                )
            )
            true
        } else {
            var mergedMonthContent = ""
            ingestDayDraft(
                txtStorageGateway = txtStorageGateway,
                monthContent = sessionState.allDraftState.draftText,
                dayMarker = dayMarker,
                dayDraftBody = sessionState.dayDraftState.draftText,
                onMergedMonthContent = {
                    mergedMonthContent = it
                    onMergedMonthContent(it)
                },
                onSaveHistoryFile = onSaveHistoryFile
            ).also { wasSuccessful ->
                if (wasSuccessful) {
                    sessionController.state = TxtEditorSessionReducer.hideEditor(
                        TxtEditorSessionReducer.markDayDraftPersisted(
                            TxtEditorSessionReducer.markAllDraftPersisted(
                                sessionState,
                                mergedMonthContent
                            )
                        )
                    )
                }
            }
        }
    }

    suspend fun openDayEditor(
        sessionController: TxtEditorSessionController,
        selectedHistoryFile: String,
        selectedMonth: String,
        logicalDayTarget: RecordLogicalDayTarget,
        fallbackMonthContent: String,
        persistedMonthContent: String,
        currentResolveResult: TxtDayBlockResolveResult,
        onMonthContentReconciled: (String) -> Unit
    ) {
        val strategy = determineOpenDayPreparationStrategy(
            sessionMonthContent = sessionController.currentMonthContent(fallbackMonthContent),
            editableMonthContent = fallbackMonthContent,
            persistedMonthContent = persistedMonthContent,
            currentResolveResult = currentResolveResult
        )
        val prepared = if (!strategy.requiresMarkerReload) {
            TxtEditableDayBlockResult(
                resolveResult = requireNotNull(strategy.resolveResult),
                monthContent = strategy.monthContent,
                canEdit = requireNotNull(strategy.resolveResult).canSave
            )
        } else {
            val normalizedDayMarker = loadDefaultDayMarker(
                selectedMonth = selectedMonth,
                logicalDayTarget = logicalDayTarget
            )
            sessionController.applyAutoDayMarker(
                selectedHistoryFile = selectedHistoryFile,
                selectedMonth = selectedMonth,
                logicalDayTarget = logicalDayTarget,
                normalizedDayMarker = normalizedDayMarker
            )
            prepareEditableDayBlock(
                monthContent = strategy.monthContent,
                dayMarker = normalizedDayMarker,
                selectedMonth = selectedMonth
            )
        }
        if (prepared.monthContent != strategy.monthContent) {
            onMonthContentReconciled(prepared.monthContent)
        }
        sessionController.syncResolvedDayBody(prepared.resolveResult.dayBody)
        sessionController.openEditor(prepared.resolveResult.dayBody)
    }
}

internal data class TxtEditableDayBlockResult(
    val resolveResult: TxtDayBlockResolveResult,
    val monthContent: String,
    val canEdit: Boolean
)

internal data class OpenDayPreparationStrategy(
    val monthContent: String,
    val resolveResult: TxtDayBlockResolveResult?,
    val requiresMarkerReload: Boolean
)

internal fun appendEmptyDayBlock(
    monthContent: String,
    dayMarker: String
): String {
    val normalizedContent = monthContent.trimEnd()
    val dayMarkerLine = buildDayMarkerLine(dayMarker)
    return if (normalizedContent.isEmpty()) {
        "$dayMarkerLine\n"
    } else {
        "$normalizedContent\n\n$dayMarkerLine\n"
    }
}

// The editor/runtime API keeps day identity as MMDD. Only raw TXT day-block
// headers add "d" so a line like 1921 remains an HHMM event, not a day marker.
private fun buildDayMarkerLine(dayMarker: String): String = "d$dayMarker"

private fun hasCanonicalMonthHeaders(content: String): Boolean {
    val lines = content
        .lineSequence()
        .map { it.trim() }
        .filter { it.isNotEmpty() }
        .take(2)
        .toList()
    if (lines.size < 2) {
        return false
    }
    return Regex("""^y\d{4}$""").matches(lines[0]) &&
        Regex("""^m(0[1-9]|1[0-2])$""").matches(lines[1])
}

// Cold-start TXT state can briefly contain a short in-memory placeholder before the
// file-backed month text arrives. Prefer the first candidate that already proves it is a
// canonical month TXT so DAY resolve/open never runs against placeholder content.
internal fun choosePreferredMonthContent(
    sessionMonthContent: String,
    editableMonthContent: String,
    persistedMonthContent: String
): String {
    if (hasCanonicalMonthHeaders(sessionMonthContent)) {
        return sessionMonthContent
    }
    if (hasCanonicalMonthHeaders(editableMonthContent)) {
        return editableMonthContent
    }
    if (hasCanonicalMonthHeaders(persistedMonthContent)) {
        return persistedMonthContent
    }
    return when {
        sessionMonthContent.isNotBlank() -> sessionMonthContent
        editableMonthContent.isNotBlank() -> editableMonthContent
        else -> persistedMonthContent
    }
}

// DAY opening has two stable branches:
// 1) if the current resolve result already points at a valid ISO day inside the selected month,
//    reuse it directly;
// 2) otherwise reopen from a canonical month TXT source and reload the default marker first.
// This keeps the state-source policy testable without pulling Compose/UI concerns into the tests.
internal fun determineOpenDayPreparationStrategy(
    sessionMonthContent: String,
    editableMonthContent: String,
    persistedMonthContent: String,
    currentResolveResult: TxtDayBlockResolveResult
): OpenDayPreparationStrategy {
    val monthContent = choosePreferredMonthContent(
        sessionMonthContent = sessionMonthContent,
        editableMonthContent = editableMonthContent,
        persistedMonthContent = persistedMonthContent
    )
    return if (currentResolveResult.dayContentIsoDate != null) {
        OpenDayPreparationStrategy(
            monthContent = monthContent,
            resolveResult = currentResolveResult,
            requiresMarkerReload = false
        )
    } else {
        OpenDayPreparationStrategy(
            monthContent = monthContent,
            resolveResult = null,
            requiresMarkerReload = true
        )
    }
}
