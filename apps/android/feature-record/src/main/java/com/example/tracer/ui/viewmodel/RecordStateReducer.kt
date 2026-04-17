package com.example.tracer

import java.time.ZoneId
import kotlin.math.roundToInt

internal object RecordStateReducer {
    fun onRecordContentChange(state: RecordUiState, value: String): RecordUiState =
        state.copy(recordContent = value)

    fun onRecordRemarkChange(state: RecordUiState, value: String): RecordUiState =
        state.copy(recordRemark = value)

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
        if (state.logicalDayIsUserOverride) {
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

    fun updateSuggestionPreferences(
        state: RecordUiState,
        lookbackDays: Int,
        topN: Int
    ): RecordUiState {
        if (state.suggestionLookbackDays == lookbackDays && state.suggestionTopN == topN) {
            return state
        }
        return state.copy(
            suggestionLookbackDays = lookbackDays,
            suggestionTopN = topN
        )
    }

    fun updateQuickActivities(state: RecordUiState, values: List<String>): RecordUiState {
        val normalized = values
            .map { it.trim() }
            .filter { it.isNotEmpty() }
            .distinct()
        // Allow an empty quick-access list so users can clear default chips that do not match
        // the currently imported alias config before rebuilding their own list.
        if (state.quickActivities == normalized) {
            return state
        }
        return state.copy(quickActivities = normalized)
    }

    fun updateAssistUiState(
        state: RecordUiState,
        assistExpanded: Boolean,
        assistSettingsExpanded: Boolean
    ): RecordUiState {
        if (
            state.assistExpanded == assistExpanded &&
            state.assistSettingsExpanded == assistSettingsExpanded
        ) {
            return state
        }
        return state.copy(
            assistExpanded = assistExpanded,
            assistSettingsExpanded = assistSettingsExpanded
        )
    }

    fun hideSuggestions(state: RecordUiState): RecordUiState =
        state.copy(suggestionsVisible = false)

    fun showSuggestionsLoading(state: RecordUiState): RecordUiState =
        state.copy(
            suggestionsVisible = true,
            isSuggestionsLoading = true,
            statusText = "Loading activity suggestions..."
        )

    fun applySuggestedActivity(state: RecordUiState, activityName: String): RecordUiState =
        state.copy(recordContent = activityName)

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
        event: FileCryptoProgressEvent,
        operationTextOverride: String? = null,
        phaseTextOverride: String? = null,
        overallProgressOverride: Float? = null,
        overallTextOverride: String? = null,
        currentTextOverride: String? = null,
        currentProgressOverride: Float? = null
    ): RecordUiState {
        val phaseText = when (event.phase) {
            FileCryptoPhase.COMPLETED -> "完成"
            FileCryptoPhase.CANCELLED -> "已取消"
            FileCryptoPhase.FAILED -> "失败"
            else -> "处理中"
        }
        val statusText = phaseText

        val overallProgress = (overallProgressOverride ?: event.overallProgressFraction)
            .coerceIn(0f, 1f)
        val currentProgress = (currentProgressOverride ?: event.currentFileProgressFraction)
            .coerceIn(0f, 1f)

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
                currentText = currentTextOverride ?: defaultCurrentText,
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
        // session so Record and TXT tabs stay aligned on the same target-day intent.
        if (state.logicalDayTarget == target && state.logicalDayIsUserOverride) {
            return state
        }
        return state.copy(
            logicalDayTarget = target,
            logicalDayIsUserOverride = true
        )
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

    private fun formatEta(etaSeconds: Long, remainingBytes: Long, phase: FileCryptoPhase): String {
        if (phase == FileCryptoPhase.COMPLETED || remainingBytes <= 0L) {
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
        phase: FileCryptoPhase,
        fallbackEtaSeconds: Long,
        fallbackRemainingBytes: Long
    ): String {
        if (phase == FileCryptoPhase.COMPLETED) {
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
