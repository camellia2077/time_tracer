package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.Spacer
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.hapticfeedback.HapticFeedbackType
import androidx.compose.ui.platform.LocalHapticFeedback
import androidx.compose.ui.unit.dp
import kotlinx.coroutines.delay
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale

private val DISPLAY_TIME_FORMATTER = SimpleDateFormat("yyyy-MM-dd HH:mm:ss", Locale.US)
private const val MAX_QUICK_ACTIVITY_COUNT = 12
private val RECORD_PRIMARY_SECTION_TOP_SPACER = 96.dp

@Composable
fun RecordSection(
    txtStorageGateway: TxtStorageGateway,
    recordContent: String,
    onRecordContentChange: (String) -> Unit,
    recordRemark: String,
    onRecordRemarkChange: (String) -> Unit,
    quickActivities: List<String>,
    availableActivityNames: List<String>,
    onQuickActivitiesUpdate: (List<String>) -> Boolean,
    assistExpanded: Boolean,
    assistSettingsExpanded: Boolean,
    onToggleAssist: () -> Unit,
    onToggleAssistSettings: () -> Unit,
    suggestionLookbackDays: Int,
    suggestionTopN: Int,
    onSuggestionLookbackDaysChange: (String) -> Unit,
    onSuggestionTopNChange: (String) -> Unit,
    suggestedActivities: List<String>,
    suggestionsVisible: Boolean,
    isSuggestionsLoading: Boolean,
    isTxtPreviewVisible: Boolean,
    isTxtPreviewLoading: Boolean,
    txtPreviewStatusText: String,
    selectedMonth: String,
    selectedHistoryFile: String,
    editableHistoryContent: String,
    logicalDayTarget: RecordLogicalDayTarget,
    onSelectLogicalDayYesterday: () -> Unit,
    onSelectLogicalDayToday: () -> Unit,
    onRefreshLogicalDayDefault: (Long) -> Unit,
    onToggleSuggestions: () -> Unit,
    onSuggestedActivityClick: (String) -> Unit,
    onOpenTxtPreview: () -> Unit,
    onDismissTxtPreview: () -> Unit,
    onRecordNow: () -> Unit
) {
    var currentTimeText by remember { mutableStateOf(formatCurrentTime(System.currentTimeMillis())) }
    var quickActivitySearch by remember { mutableStateOf("") }
    val haptic = LocalHapticFeedback.current

    // Keep the displayed clock live and let the state layer auto-switch the activity-day default
    // at 06:00 whenever the user has not explicitly overridden yesterday/today on this visit.
    LaunchedEffect(Unit) {
        while (true) {
            val now = System.currentTimeMillis()
            currentTimeText = formatCurrentTime(now)
            onRefreshLogicalDayDefault(now)
            delay(1000)
        }
    }

    Column(
        modifier = Modifier.fillMaxWidth(),
        verticalArrangement = Arrangement.spacedBy(16.dp)
    ) {
        if (suggestionsVisible) {
            RecordSuggestionsSection(
                suggestionsVisible = true,
                isSuggestionsLoading = isSuggestionsLoading,
                suggestedActivities = suggestedActivities,
                onToggleSuggestions = onToggleSuggestions,
                onSuggestedActivityClick = onSuggestedActivityClick,
                showToggleButton = false
            )
        } else {
            Spacer(
                modifier = Modifier
                    .fillMaxWidth()
                    .height(RECORD_PRIMARY_SECTION_TOP_SPACER)
            )
        }

        RecordTimeSettingsCard(
            currentTimeText = currentTimeText,
            logicalDayTarget = logicalDayTarget,
            onSelectLogicalDayYesterday = onSelectLogicalDayYesterday,
            onSelectLogicalDayToday = onSelectLogicalDayToday
        )

        RecordQuickAccessCard(
            recordContent = recordContent,
            onRecordContentChange = onRecordContentChange,
            quickActivities = quickActivities,
            availableActivityNames = availableActivityNames,
            onQuickActivitiesUpdate = onQuickActivitiesUpdate,
            assistSettingsExpanded = assistSettingsExpanded,
            onToggleAssistSettings = onToggleAssistSettings,
            suggestionLookbackDays = suggestionLookbackDays,
            suggestionTopN = suggestionTopN,
            onSuggestionLookbackDaysChange = onSuggestionLookbackDaysChange,
            onSuggestionTopNChange = onSuggestionTopNChange,
            quickActivitySearch = quickActivitySearch,
            onQuickActivitySearchChange = { quickActivitySearch = it },
            maxQuickActivityCount = MAX_QUICK_ACTIVITY_COUNT
        )

        RecordInputCard(
            recordContent = recordContent,
            onRecordContentChange = onRecordContentChange,
            recordRemark = recordRemark,
            onRecordRemarkChange = onRecordRemarkChange,
            suggestionsVisible = suggestionsVisible,
            onToggleSuggestions = onToggleSuggestions,
            onOpenTxtPreview = onOpenTxtPreview
        ) {
            haptic.performHapticFeedback(HapticFeedbackType.LongPress)
            onRecordNow()
        }
    }

    if (isTxtPreviewVisible) {
        RecordTxtPreviewSheet(
            txtStorageGateway = txtStorageGateway,
            selectedMonth = selectedMonth,
            selectedHistoryFile = selectedHistoryFile,
            editableHistoryContent = editableHistoryContent,
            logicalDayTarget = logicalDayTarget,
            isLoading = isTxtPreviewLoading,
            previewStatusText = txtPreviewStatusText,
            onDismissRequest = onDismissTxtPreview
        )
    }
}

private fun formatCurrentTime(currentTimeMillis: Long): String =
    DISPLAY_TIME_FORMATTER.format(Date(currentTimeMillis))
