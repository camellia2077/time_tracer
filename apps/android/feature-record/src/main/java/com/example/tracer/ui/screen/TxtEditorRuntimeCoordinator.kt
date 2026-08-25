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
        return applyDayEditAndPersist(
            monthContent = monthContent,
            dayMarker = dayMarker,
            selectedMonth = selectedMonth,
            dayRemark = dayRemark,
            events = events,
            onMergedMonthContent = onMergedMonthContent,
            onSaveHistoryFile = onSaveHistoryFile
        ).ok
    }

    suspend fun replaceDayActivityToken(
        monthContent: String,
        dayMarker: String,
        selectedMonth: String,
        dayRemark: String,
        events: List<TxtDayEditEvent>,
        sourceActivityToken: String,
        targetActivityToken: String,
        onMergedMonthContent: (String) -> Unit,
        onSaveHistoryFile: () -> Unit
    ): TxtDayActivityReplacementResult {
        if (sourceActivityToken.isBlank()) {
            return TxtDayActivityReplacementResult(
                ok = false,
                replacedEventCount = 0,
                message = "source activity is required."
            )
        }
        if (targetActivityToken.isBlank()) {
            return TxtDayActivityReplacementResult(
                ok = false,
                replacedEventCount = 0,
                message = "replacement activity is required."
            )
        }
        if (sourceActivityToken == targetActivityToken) {
            return TxtDayActivityReplacementResult(
                ok = false,
                replacedEventCount = 0,
                message = "source and replacement activities are identical."
            )
        }

        val replacedEventCount = events.count { it.activityToken == sourceActivityToken }
        if (replacedEventCount == 0) {
            return TxtDayActivityReplacementResult(
                ok = false,
                replacedEventCount = 0,
                message = "selected activity is not present in this day."
            )
        }

        val replacedEvents = events.map { event ->
            if (event.activityToken == sourceActivityToken) {
                event.copy(activityToken = targetActivityToken)
            } else {
                event
            }
        }
        val applied = applyDayEditAndPersist(
            monthContent = monthContent,
            dayMarker = dayMarker,
            selectedMonth = selectedMonth,
            dayRemark = dayRemark,
            events = replacedEvents,
            onMergedMonthContent = onMergedMonthContent,
            onSaveHistoryFile = onSaveHistoryFile
        )
        return TxtDayActivityReplacementResult(
            ok = applied.ok,
            replacedEventCount = if (applied.ok) replacedEventCount else 0,
            message = applied.message
        )
    }

    suspend fun prepareMonthActivityEdits(
        monthContent: String,
        selectedMonth: String
    ): TxtMonthActivityEditsResult {
        val dayMarkers = monthContent.lineSequence()
            .mapNotNull { line -> MONTH_DAY_MARKER.matchEntire(line)?.groupValues?.get(1) }
            .toList()
        val dayEdits = mutableListOf<TxtMonthDayEdit>()
        for (dayMarker in dayMarkers) {
            val resolved = resolveDayEdit(
                monthContent = monthContent,
                dayMarker = dayMarker,
                selectedMonth = selectedMonth
            )
            if (!resolved.ok) {
                return TxtMonthActivityEditsResult(
                    ok = false,
                    snapshot = null,
                    message = resolved.message
                )
            }
            if (resolved.found && resolved.canSave) {
                dayEdits += TxtMonthDayEdit(
                    dayMarker = resolved.normalizedDayMarker,
                    dayRemark = resolved.dayRemark,
                    events = resolved.events
                )
            }
        }
        return TxtMonthActivityEditsResult(
            ok = true,
            snapshot = TxtMonthActivityEditSnapshot(
                monthContent = monthContent,
                selectedMonth = selectedMonth,
                dayEdits = dayEdits
            ),
            message = ""
        )
    }

    suspend fun replaceMonthActivityToken(
        snapshot: TxtMonthActivityEditSnapshot,
        sourceActivityToken: String,
        targetActivityToken: String,
        onMergedMonthContent: (String) -> Unit,
        onSaveHistoryFile: () -> Unit
    ): TxtMonthActivityReplacementResult {
        if (sourceActivityToken.isBlank()) {
            return TxtMonthActivityReplacementResult(false, 0, "source activity is required.")
        }
        if (targetActivityToken.isBlank()) {
            return TxtMonthActivityReplacementResult(false, 0, "replacement activity is required.")
        }
        if (sourceActivityToken == targetActivityToken) {
            return TxtMonthActivityReplacementResult(
                false,
                0,
                "source and replacement activities are identical."
            )
        }

        val replacedEventCount = snapshot.dayEdits.sumOf { dayEdit ->
            dayEdit.events.count { it.activityToken == sourceActivityToken }
        }
        if (replacedEventCount == 0) {
            return TxtMonthActivityReplacementResult(
                false,
                0,
                "selected activity is not present in this month."
            )
        }

        var updatedMonthContent = snapshot.monthContent
        for (dayEdit in snapshot.dayEdits) {
            if (dayEdit.events.none { it.activityToken == sourceActivityToken }) {
                continue
            }
            val applied = txtStorageGateway.applyTxtDayEdit(
                content = updatedMonthContent,
                dayMarker = dayEdit.dayMarker,
                selectedMonth = snapshot.selectedMonth,
                dayRemark = dayEdit.dayRemark,
                events = dayEdit.events.map { event ->
                    if (event.activityToken == sourceActivityToken) {
                        event.copy(activityToken = targetActivityToken)
                    } else {
                        event
                    }
                }
            )
            if (!applied.ok) {
                return TxtMonthActivityReplacementResult(false, 0, applied.message)
            }
            updatedMonthContent = applied.updatedContent
        }
        onMergedMonthContent(updatedMonthContent)
        onSaveHistoryFile()
        return TxtMonthActivityReplacementResult(true, replacedEventCount, "")
    }

    private suspend fun applyDayEditAndPersist(
        monthContent: String,
        dayMarker: String,
        selectedMonth: String,
        dayRemark: String,
        events: List<TxtDayEditEvent>,
        onMergedMonthContent: (String) -> Unit,
        onSaveHistoryFile: () -> Unit
    ): TxtDayEditApplyResult {
        val applied = txtStorageGateway.applyTxtDayEdit(
            content = monthContent,
            dayMarker = dayMarker,
            selectedMonth = selectedMonth,
            dayRemark = dayRemark,
            events = events
        )
        if (!applied.ok) {
            return applied
        }
        onMergedMonthContent(applied.updatedContent)
        onSaveHistoryFile()
        return applied
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

internal data class TxtDayActivityReplacementResult(
    val ok: Boolean,
    val replacedEventCount: Int,
    val message: String
)

internal data class TxtMonthDayEdit(
    val dayMarker: String,
    val dayRemark: String,
    val events: List<TxtDayEditEvent>
)

internal data class TxtMonthActivityEditSnapshot(
    val monthContent: String,
    val selectedMonth: String,
    val dayEdits: List<TxtMonthDayEdit>
)

internal data class TxtMonthActivityEditsResult(
    val ok: Boolean,
    val snapshot: TxtMonthActivityEditSnapshot?,
    val message: String
)

internal data class TxtMonthActivityReplacementResult(
    val ok: Boolean,
    val replacedEventCount: Int,
    val message: String
)

private val MONTH_DAY_MARKER = Regex("d(\\d{4})")

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
