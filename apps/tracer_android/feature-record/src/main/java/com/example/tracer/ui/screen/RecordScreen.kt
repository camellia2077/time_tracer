package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
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

@Composable
fun RecordSection(
    recordContent: String,
    onRecordContentChange: (String) -> Unit,
    recordRemark: String,
    onRecordRemarkChange: (String) -> Unit,
    quickActivities: List<String>,
    availableActivityNames: List<String>,
    onQuickActivitiesUpdate: (List<String>) -> Unit,
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
    useManualDate: Boolean,
    manualDate: String,
    onUseAutoDate: () -> Unit,
    onUseManualDate: () -> Unit,
    onManualDateChange: (String) -> Unit,
    onToggleSuggestions: () -> Unit,
    onSuggestedActivityClick: (String) -> Unit,
    onRecordNow: () -> Unit
) {
    var currentTimeText by remember { mutableStateOf(formatCurrentTime()) }
    var quickActivitySearch by remember { mutableStateOf("") }
    val haptic = LocalHapticFeedback.current

    // Keep the time header live while the screen is active.
    LaunchedEffect(Unit) {
        while (true) {
            currentTimeText = formatCurrentTime()
            delay(1000)
        }
    }

    Column(
        modifier = Modifier.fillMaxWidth(),
        verticalArrangement = Arrangement.spacedBy(16.dp)
    ) {
        RecordTimeSettingsCard(
            useManualDate = useManualDate,
            manualDate = manualDate,
            currentTimeText = currentTimeText,
            onUseAutoDate = onUseAutoDate,
            onUseManualDate = onUseManualDate,
            onManualDateChange = onManualDateChange
        )

        RecordInputCard(
            recordContent = recordContent,
            onRecordContentChange = onRecordContentChange,
            recordRemark = recordRemark,
            onRecordRemarkChange = onRecordRemarkChange
        ) {
            haptic.performHapticFeedback(HapticFeedbackType.LongPress)
            onRecordNow()
        }

        Column(verticalArrangement = Arrangement.spacedBy(16.dp)) {
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

            RecordSuggestionsSection(
                suggestionsVisible = suggestionsVisible,
                isSuggestionsLoading = isSuggestionsLoading,
                suggestedActivities = suggestedActivities,
                onToggleSuggestions = onToggleSuggestions,
                onSuggestedActivityClick = onSuggestedActivityClick
            )
        }
    }
}

private fun formatCurrentTime(): String = DISPLAY_TIME_FORMATTER.format(Date())
