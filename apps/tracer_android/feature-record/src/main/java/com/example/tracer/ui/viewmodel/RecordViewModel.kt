package com.example.tracer

import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.lifecycle.ViewModel
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.launch
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale
import kotlin.math.roundToInt

data class CryptoProgressUiState(
    val isVisible: Boolean = false,
    val operationText: String = "",
    val phaseText: String = "",
    val statusText: String = "",
    val overallProgress: Float = 0f,
    val overallText: String = "",
    val currentProgress: Float = 0f,
    val currentText: String = "",
    val detailsText: String = "",
    val advancedDetailsText: String = "",
    val startedAtEpochMs: Long = 0L
)

data class RecordUiState(
    val recordContent: String = "",
    val recordRemark: String = "",
    val useManualDate: Boolean = false,
    val manualDate: String = currentIsoDate(),
    val historyFiles: List<String> = emptyList(),
    val availableMonths: List<String> = emptyList(),
    val selectedMonth: String = "",
    val selectedHistoryFile: String = "",
    val selectedHistoryContent: String = "",
    val editableHistoryContent: String = "",
    val quickActivities: List<String> = listOf("meal", "洗漱", "上厕所"),
    val assistExpanded: Boolean = false,
    val assistSettingsExpanded: Boolean = false,
    val suggestionLookbackDays: Int = 7,
    val suggestionTopN: Int = 5,
    val suggestedActivities: List<String> = emptyList(),
    val suggestionsVisible: Boolean = false,
    val isSuggestionsLoading: Boolean = false,
    val statusText: String = "",
    val cryptoProgress: CryptoProgressUiState = CryptoProgressUiState()
)

private fun currentIsoDate(): String =
    SimpleDateFormat("yyyy-MM-dd", Locale.US).format(Date())

class RecordViewModel(private val recordUseCases: RecordUseCases) : ViewModel() {
    var uiState by mutableStateOf(RecordUiState())
        private set

    fun onRecordContentChange(value: String) {
        uiState = uiState.copy(recordContent = value)
    }

    fun onRecordRemarkChange(value: String) {
        uiState = uiState.copy(recordRemark = value)
    }

    fun useAutoDate() {
        uiState = uiState.copy(useManualDate = false)
    }

    fun useManualDate() {
        uiState = uiState.copy(useManualDate = true)
    }

    fun onManualDateChange(value: String) {
        uiState = uiState.copy(manualDate = value)
    }

    fun updateEditableHistoryContent(value: String) {
        uiState = uiState.copy(editableHistoryContent = value)
    }

    fun updateSuggestionPreferences(lookbackDays: Int, topN: Int) {
        if (uiState.suggestionLookbackDays == lookbackDays && uiState.suggestionTopN == topN) {
            return
        }

        uiState = uiState.copy(
            suggestionLookbackDays = lookbackDays,
            suggestionTopN = topN
        )
    }

    fun updateQuickActivities(values: List<String>) {
        val normalized = values
            .map { it.trim() }
            .filter { it.isNotEmpty() }
            .distinct()
        if (normalized.isEmpty() || uiState.quickActivities == normalized) {
            return
        }
        uiState = uiState.copy(quickActivities = normalized)
    }

    fun updateAssistUiState(assistExpanded: Boolean, assistSettingsExpanded: Boolean) {
        if (uiState.assistExpanded == assistExpanded &&
            uiState.assistSettingsExpanded == assistSettingsExpanded
        ) {
            return
        }
        uiState = uiState.copy(
            assistExpanded = assistExpanded,
            assistSettingsExpanded = assistSettingsExpanded
        )
    }

    fun toggleSuggestions() {
        if (uiState.suggestionsVisible) {
            uiState = uiState.copy(suggestionsVisible = false)
            return
        }

        uiState = uiState.copy(
            suggestionsVisible = true,
            isSuggestionsLoading = true,
            statusText = "Loading activity suggestions..."
        )
        viewModelScope.launch {
            val resultState = recordUseCases.loadActivitySuggestions(
                state = uiState,
                lookbackDays = uiState.suggestionLookbackDays,
                topN = uiState.suggestionTopN
            )
            uiState = uiState.copy(
                suggestedActivities = resultState.suggestedActivities,
                isSuggestionsLoading = resultState.isSuggestionsLoading,
                statusText = resultState.statusText
            )
        }
    }

    fun applySuggestedActivity(activityName: String) {
        uiState = uiState.copy(recordContent = activityName)
    }

    fun setStatusText(message: String) {
        uiState = uiState.copy(statusText = message)
    }

    fun startCryptoProgress(operationText: String) {
        uiState = uiState.copy(
            cryptoProgress = CryptoProgressUiState(
                isVisible = true,
                operationText = operationText,
                phaseText = "处理中",
                statusText = "处理中",
                detailsText = "已用时 00:00 | 预计剩余 --:--",
                startedAtEpochMs = System.currentTimeMillis()
            )
        )
    }

    fun updateCryptoProgress(
        event: FileCryptoProgressEvent,
        operationTextOverride: String? = null,
        overallProgressOverride: Float? = null,
        overallTextOverride: String? = null,
        currentTextOverride: String? = null,
        currentProgressOverride: Float? = null
    ) {
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
        val startedAtEpochMs = uiState.cryptoProgress.startedAtEpochMs
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

        uiState = uiState.copy(
            cryptoProgress = uiState.cryptoProgress.copy(
                isVisible = true,
                operationText = operationTextOverride ?: uiState.cryptoProgress.operationText,
                phaseText = phaseText,
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

    fun finishCryptoProgress(statusText: String, keepVisible: Boolean) {
        uiState = uiState.copy(
            cryptoProgress = uiState.cryptoProgress.copy(
                isVisible = keepVisible,
                phaseText = statusText,
                statusText = statusText
            )
        )
    }

    fun clearCryptoProgress() {
        uiState = uiState.copy(cryptoProgress = CryptoProgressUiState())
    }

    fun recordNow() {
        viewModelScope.launch {
            uiState = recordUseCases.recordNow(uiState)
        }
    }

    fun refreshHistory() {
        viewModelScope.launch {
            uiState = recordUseCases.refreshHistory(uiState)
        }
    }

    fun openHistoryFile(path: String) {
        viewModelScope.launch {
            uiState = recordUseCases.openHistoryFile(uiState, path)
        }
    }

    fun openMonth(month: String) {
        viewModelScope.launch {
            uiState = recordUseCases.openMonth(uiState, month)
        }
    }

    fun openPreviousMonth() {
        viewModelScope.launch {
            uiState = recordUseCases.openPreviousMonth(uiState)
        }
    }

    fun openNextMonth() {
        viewModelScope.launch {
            uiState = recordUseCases.openNextMonth(uiState)
        }
    }

    fun saveHistoryFileAndSync() {
        viewModelScope.launch {
            uiState = recordUseCases.saveHistoryFileAndSync(uiState)
        }
    }

    fun discardUnsavedHistoryDraft() {
        if (uiState.editableHistoryContent == uiState.selectedHistoryContent) {
            return
        }
        uiState = uiState.copy(
            editableHistoryContent = uiState.selectedHistoryContent
        )
    }

    fun clearTxtEditorState() {
        uiState = recordUseCases.clearEditorState(uiState)
    }
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

class RecordViewModelFactory(
    private val recordGateway: RecordGateway,
    private val txtStorageGateway: TxtStorageGateway,
    private val queryGateway: QueryGateway
) : ViewModelProvider.Factory {
    override fun <T : ViewModel> create(modelClass: Class<T>): T {
        if (modelClass.isAssignableFrom(RecordViewModel::class.java)) {
            @Suppress("UNCHECKED_CAST")
            return RecordViewModel(
                RecordUseCases(
                    recordGateway = recordGateway,
                    txtStorageGateway = txtStorageGateway,
                    queryGateway = queryGateway
                )
            ) as T
        }
        throw IllegalArgumentException("Unknown ViewModel class: ${modelClass.name}")
    }
}
