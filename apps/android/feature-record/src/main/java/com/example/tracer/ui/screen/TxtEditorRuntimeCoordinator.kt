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
}
