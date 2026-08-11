package com.example.tracer

import java.time.ZoneId
import kotlin.math.roundToInt

internal object RecordStateReducer {
    fun hydratePersistedRecordInput(
        state: RecordUiState,
        persistedInput: PersistedRecordInputSnapshot
    ): RecordUiState {
        val draft = persistedInput.draft
        return if (draft != null) {
            state.copy(
                authoringMode = persistedInput.lastAuthoringMode,
                txtOutputMode = persistedInput.lastTxtOutputMode,
                recordContent = draft.recordContent,
                recordRemark = draft.recordRemark,
                intervalStart = draft.intervalStart,
                intervalEnd = draft.intervalEnd,
                intervalStartedAtEpochMs = draft.intervalStartedAtEpochMs,
                attributionDateIso = draft.attributionDateIso,
                logicalDayTarget = draft.logicalDayTarget
            )
        } else {
            state.copy(
                authoringMode = persistedInput.lastAuthoringMode,
                txtOutputMode = persistedInput.lastTxtOutputMode
            )
        }
    }

    fun onAuthoringModeChange(state: RecordUiState, value: RecordAuthoringMode): RecordUiState =
        state.copy(authoringMode = value)

    fun onRecordContentChange(state: RecordUiState, value: String): RecordUiState =
        state.copy(recordContent = value)

    fun onRecordRemarkChange(state: RecordUiState, value: String): RecordUiState =
        state.copy(recordRemark = value)

    fun onIntervalStartChange(state: RecordUiState, value: String): RecordUiState =
        state.copy(intervalStart = value)

    fun onIntervalEndChange(state: RecordUiState, value: String): RecordUiState =
        state.copy(intervalEnd = value)

    fun selectLogicalDayYesterday(state: RecordUiState): RecordUiState =
        selectLogicalDayTarget(state, RecordLogicalDayTarget.YESTERDAY)

    fun selectLogicalDayToday(state: RecordUiState): RecordUiState =
        selectLogicalDayTarget(state, RecordLogicalDayTarget.TODAY)

    fun refreshLogicalDayDefault(
        state: RecordUiState,
        currentTimeMillis: Long,
        logicalDayZoneId: ZoneId
    ): RecordUiState {
        // Record uses an activity-day concept instead of the natural day: before 06:00 we still
        // default to "yesterday" so late-night work keeps appending to the previous day's block.
        // The zone is injected from the Android session clock instead of reading the host system
        // default implicitly, so runtime behavior stays device-local while tests remain stable.
        if (state.logicalDayIsUserOverride || hasPersistableRecordDraft(state)) {
            return state
        }
        val defaultTarget = defaultLogicalDayTarget(
            currentTimeMillis = currentTimeMillis,
            zoneId = logicalDayZoneId
        )
        if (state.logicalDayTarget == defaultTarget) {
            return state
        }
        return state.copy(logicalDayTarget = defaultTarget)
    }

    fun updateEditableHistoryContent(state: RecordUiState, value: String): RecordUiState {
        val selectedFile = state.selectedHistoryFile
        if (selectedFile.isBlank()) {
            return state.copy(editableHistoryContent = value)
        }

        val nextDrafts = state.historyDraftsByFile.toMutableMap()
        if (value == state.selectedHistoryContent) {
            nextDrafts.remove(selectedFile)
        } else {
            nextDrafts[selectedFile] = value
        }
        return state.copy(
            editableHistoryContent = value,
            historyDraftsByFile = nextDrafts
        )
    }

    fun updateFrequentPreferences(
        state: RecordUiState,
        lookbackDays: Int,
        topN: Int
    ): RecordUiState {
        if (state.frequentLookbackDays == lookbackDays && state.frequentTopN == topN) {
            return state
        }
        return state.copy(
            frequentLookbackDays = lookbackDays,
            frequentTopN = topN
        )
    }

    fun updateFrequentOutputMode(
        state: RecordUiState,
        value: RecordFrequentOutputMode
    ): RecordUiState {
        if (state.frequentOutputMode == value) {
            return state
        }
        return state.copy(frequentOutputMode = value)
    }

    fun updateCanonicalCatalogDisplayMode(
        state: RecordUiState,
        value: RecordFrequentOutputMode
    ): RecordUiState {
        if (state.canonicalCatalogDisplayMode == value) {
            return state
        }
        return state.copy(canonicalCatalogDisplayMode = value)
    }

    fun updateQuickActivities(state: RecordUiState, values: List<String>): RecordUiState {
        val normalized = values
            .map { it.trim() }
            .filter { it.isNotEmpty() }
            .distinct()
        // Allow an empty quick-access list so users can clear default chips that do not match
        // the currently imported canonical config before rebuilding their own list.
        if (state.quickActivities == normalized) {
            return state
        }
        return state.copy(quickActivities = normalized)
    }

    fun updateActualTimeExpanded(
        state: RecordUiState,
        expanded: Boolean
    ): RecordUiState {
        if (state.actualTimeExpanded == expanded) {
            return state
        }
        return state.copy(actualTimeExpanded = expanded)
    }

    fun updateQuickAccessCardExpanded(
        state: RecordUiState,
        expanded: Boolean
    ): RecordUiState {
        if (state.quickAccessCardExpanded == expanded) {
            return state
        }
        return state.copy(quickAccessCardExpanded = expanded)
    }

    fun updateAssistUiState(
        state: RecordUiState,
        assistSettingsExpanded: Boolean
    ): RecordUiState {
        if (state.assistSettingsExpanded == assistSettingsExpanded) {
            return state
        }
        return state.copy(
            assistSettingsExpanded = assistSettingsExpanded
        )
    }

    fun updateCollapsedCanonicalRootPaths(
        state: RecordUiState,
        paths: Set<String>
    ): RecordUiState {
        val normalized = paths
            .map { it.trim() }
            .filter { it.isNotEmpty() }
            .toSet()
        if (state.collapsedCanonicalRootPaths == normalized) {
            return state
        }
        return state.copy(collapsedCanonicalRootPaths = normalized)
    }

    fun updateOrderedCanonicalRootPaths(
        state: RecordUiState,
        paths: List<String>
    ): RecordUiState {
        val normalized = paths
            .map { it.trim() }
            .filter { it.isNotEmpty() }
            .distinct()
        if (state.orderedCanonicalRootPaths == normalized) {
            return state
        }
        return state.copy(orderedCanonicalRootPaths = normalized)
    }

    fun hideFrequentActivities(state: RecordUiState): RecordUiState =
        state.copy(frequentActivitiesVisible = false)

    fun showFrequentActivitiesLoading(state: RecordUiState): RecordUiState =
        state.copy(
            frequentActivitiesVisible = true,
            isFrequentActivitiesLoading = true,
            canonicalCatalogStatusText = "",
            statusText = "Loading frequent activities..."
        )

    fun hideCanonicalCatalog(state: RecordUiState): RecordUiState =
        state.copy(
            isCanonicalCatalogVisible = false,
            canonicalBrowserTarget = null,
            isCanonicalCatalogLoading = false
        )

    fun showCanonicalCatalogLoading(
        state: RecordUiState,
        target: CanonicalBrowserTarget
    ): RecordUiState =
        state.copy(
            isCanonicalCatalogVisible = true,
            canonicalBrowserTarget = target,
            isCanonicalCatalogLoading = true,
            canonicalCatalogStatusText = "",
            statusText = "Loading canonical catalog..."
        )

    fun applyFrequentActivity(state: RecordUiState, activityName: String): RecordUiState =
        state.copy(recordContent = activityName)

    fun applyCanonicalCatalogEntry(state: RecordUiState, token: String): RecordUiState =
        state.copy(
            recordContent = token.trim(),
            frequentActivitiesVisible = false,
            isCanonicalCatalogVisible = false,
            canonicalBrowserTarget = null,
            isCanonicalCatalogLoading = false,
            statusText = ""
        )

    fun setStatusText(state: RecordUiState, message: String): RecordUiState =
        state.copy(statusText = message)

    fun startCryptoProgress(state: RecordUiState, operationText: String): RecordUiState =
        state.copy(
            cryptoProgress = CryptoProgressUiState(
                isVisible = true,
                operationText = operationText,
                phaseText = "处理中",
                statusText = "处理中",
                detailsText = "已用时 00:00 | 预计剩余 --:--",
                startedAtEpochMs = System.currentTimeMillis()
            )
        )

    fun updateCryptoProgress(
        state: RecordUiState,
        event: TracerExchangeProgressEvent,
        operationTextOverride: String? = null,
        phaseTextOverride: String? = null,
        overallProgressOverride: Float? = null,
        overallTextOverride: String? = null
    ): RecordUiState {
        val phaseText = when (event.phase) {
            TracerExchangePhase.COMPLETED -> "完成"
            TracerExchangePhase.CANCELLED -> "已取消"
            TracerExchangePhase.FAILED -> "失败"
            else -> "处理中"
        }
        val statusText = phaseText

        val overallProgress = (overallProgressOverride ?: event.overallProgressFraction)
            .coerceIn(0f, 1f)
        val currentProgress = event.currentFileProgressFraction

        val defaultOverallText =
            "${(overallProgress * 100f).roundToInt()}% (${event.currentFileIndex}/${event.totalFiles})"
        val groupLabel = event.currentGroupLabel.ifBlank { "(root)" }
        val defaultCurrentText =
            "${groupLabel} ${(currentProgress * 100f).roundToInt()}% (${event.fileIndexInGroup}/${event.fileCountInGroup})"

        val nowEpochMs = System.currentTimeMillis()
        val startedAtEpochMs = state.cryptoProgress.startedAtEpochMs
        val elapsedSeconds = if (startedAtEpochMs > 0L) {
            ((nowEpochMs - startedAtEpochMs) / 1000L).coerceAtLeast(0L)
        } else {
            0L
        }
        val detailsText = buildString {
            append("已用时 ")
            append(formatDuration(elapsedSeconds))
            append(" | 预计剩余 ")
            append(
                formatBatchEta(
                    overallProgress = overallProgress,
                    elapsedSeconds = elapsedSeconds,
                    phase = event.phase,
                    fallbackEtaSeconds = event.etaSeconds,
                    fallbackRemainingBytes = event.remainingBytes
                )
            )
        }
        val advancedDetailsText = buildString {
            append("speed ")
            append(formatBytes(event.speedBytesPerSec))
            append("/s")
            append(" | remain ")
            append(formatBytes(event.remainingBytes))
        }

        return state.copy(
            cryptoProgress = state.cryptoProgress.copy(
                isVisible = true,
                operationText = operationTextOverride ?: state.cryptoProgress.operationText,
                phaseText = phaseTextOverride ?: phaseText,
                statusText = statusText,
                overallProgress = overallProgress,
                overallText = overallTextOverride ?: defaultOverallText,
                currentProgress = currentProgress,
                currentText = defaultCurrentText,
                detailsText = detailsText,
                advancedDetailsText = advancedDetailsText
            )
        )
    }

    fun finishCryptoProgress(
        state: RecordUiState,
        statusText: String,
        keepVisible: Boolean,
        detailsTextOverride: String? = null
    ): RecordUiState = state.copy(
        cryptoProgress = state.cryptoProgress.copy(
            isVisible = keepVisible,
            phaseText = statusText,
            statusText = statusText,
            detailsText = detailsTextOverride ?: state.cryptoProgress.detailsText
        )
    )

    fun clearCryptoProgress(state: RecordUiState): RecordUiState =
        state.copy(cryptoProgress = CryptoProgressUiState())

    fun showTxtPreviewLoading(state: RecordUiState): RecordUiState =
        state.copy(
            isTxtPreviewVisible = true,
            isTxtPreviewLoading = true,
            txtPreviewStatusText = ""
        )

    fun dismissTxtPreview(state: RecordUiState): RecordUiState =
        state.copy(
            isTxtPreviewVisible = false,
            isTxtPreviewLoading = false,
            txtPreviewStatusText = ""
        )

    fun discardUnsavedHistoryDraft(state: RecordUiState): RecordUiState {
        val selectedFile = state.selectedHistoryFile
        if (selectedFile.isBlank() || state.editableHistoryContent == state.selectedHistoryContent) {
            return state
        }

        val nextDrafts = state.historyDraftsByFile.toMutableMap()
        nextDrafts.remove(selectedFile)
        return state.copy(
            editableHistoryContent = state.selectedHistoryContent,
            historyDraftsByFile = nextDrafts
        )
    }

    private fun selectLogicalDayTarget(
        state: RecordUiState,
        target: RecordLogicalDayTarget
    ): RecordUiState {
        // Once users explicitly choose yesterday/today, keep that override for the current app
        // session so Record and the Config-embedded TXT editor stay aligned on the same
        // target-day intent.
        if (state.logicalDayTarget == target && state.logicalDayIsUserOverride) {
            return state
        }
        return state.copy(
            logicalDayTarget = target,
            logicalDayIsUserOverride = true
        )
    }

    internal fun hasPersistableRecordDraft(state: RecordUiState): Boolean {
        return state.recordContent.isNotBlank() ||
            state.recordRemark.isNotBlank() ||
            state.intervalStart.isNotBlank() ||
            state.intervalEnd.isNotBlank()
    }

    private fun formatBytes(bytes: Long): String {
        val normalizedBytes = bytes.coerceAtLeast(0L)
        val units = listOf("B", "KB", "MB", "GB", "TB")
        var value = normalizedBytes.toDouble()
        var index = 0
        while (value >= 1024.0 && index < units.lastIndex) {
            value /= 1024.0
            index++
        }
        if (index == 0) {
            return "${normalizedBytes}B"
        }
        val rounded = ((value * 10.0).roundToInt()) / 10.0
        return "${rounded}${units[index]}"
    }

    private fun formatEta(etaSeconds: Long, remainingBytes: Long, phase: TracerExchangePhase): String {
        if (phase == TracerExchangePhase.COMPLETED || remainingBytes <= 0L) {
            return "00:00"
        }
        if (etaSeconds <= 0L) {
            return "--:--"
        }
        val hours = etaSeconds / 3600L
        val minutes = (etaSeconds % 3600L) / 60L
        val seconds = etaSeconds % 60L
        return if (hours > 0L) {
            "%d:%02d:%02d".format(hours, minutes, seconds)
        } else {
            "%02d:%02d".format(minutes, seconds)
        }
    }

    private fun formatBatchEta(
        overallProgress: Float,
        elapsedSeconds: Long,
        phase: TracerExchangePhase,
        fallbackEtaSeconds: Long,
        fallbackRemainingBytes: Long
    ): String {
        if (phase == TracerExchangePhase.COMPLETED) {
            return "00:00"
        }

        val boundedProgress = overallProgress.coerceIn(0f, 1f)
        if (boundedProgress >= 1f) {
            return "00:00"
        }
        if (elapsedSeconds <= 0L || boundedProgress <= 0f) {
            return formatEta(fallbackEtaSeconds, fallbackRemainingBytes, phase)
        }

        val estimatedTotalSeconds =
            (elapsedSeconds.toDouble() / boundedProgress.toDouble()).toLong()
        val remainingSeconds =
            (estimatedTotalSeconds - elapsedSeconds).coerceAtLeast(0L)
        if (remainingSeconds == 0L) {
            return formatEta(fallbackEtaSeconds, fallbackRemainingBytes, phase)
        }
        return formatDuration(remainingSeconds)
    }

    private fun formatDuration(totalSeconds: Long): String {
        val safeSeconds = totalSeconds.coerceAtLeast(0L)
        val hours = safeSeconds / 3600L
        val minutes = (safeSeconds % 3600L) / 60L
        val seconds = safeSeconds % 60L
        return if (hours > 0L) {
            "%d:%02d:%02d".format(hours, minutes, seconds)
        } else {
            "%02d:%02d".format(minutes, seconds)
        }
    }
}
