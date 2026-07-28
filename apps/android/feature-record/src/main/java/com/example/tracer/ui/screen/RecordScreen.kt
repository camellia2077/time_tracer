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
import java.time.Clock
import java.util.Date
import java.util.Locale

private val DISPLAY_TIME_FORMATTER = SimpleDateFormat("yyyy-MM-dd HH:mm:ss", Locale.US)
private const val MAX_QUICK_ACTIVITY_COUNT = 12

@Composable
fun RecordSection(
    txtStorageGateway: TxtStorageGateway,
    authoringMode: RecordAuthoringMode,
    onAuthoringModeChange: (RecordAuthoringMode) -> Unit,
    recordContent: String,
    onRecordContentChange: (String) -> Unit,
    recordRemark: String,
    onRecordRemarkChange: (String) -> Unit,
    intervalStart: String,
    onIntervalStartChange: (String) -> Unit,
    intervalEnd: String,
    onIntervalEndChange: (String) -> Unit,
    intervalStartedAtEpochMs: Long,
    attributionDateIso: String,
    quickActivities: List<String>,
    availableActivityNames: List<String>,
    onQuickActivitiesUpdate: (List<String>) -> Boolean,
    quickAccessCardExpanded: Boolean = true,
    onToggleQuickAccessCard: () -> Unit = {},
    assistSettingsExpanded: Boolean,
    onToggleAssistSettings: () -> Unit,
    suggestionLookbackDays: Int,
    suggestionTopN: Int,
    onSuggestionLookbackDaysChange: (String) -> Unit,
    onSuggestionTopNChange: (String) -> Unit,
    suggestionOutputMode: RecordSuggestionOutputMode,
    onSuggestionOutputModeChange: (RecordSuggestionOutputMode) -> Unit,
    suggestedActivities: List<RecordSuggestedActivity>,
    canonicalCatalogRoots: List<CanonicalPathNode>,
    canonicalCatalogStatusText: String,
    canonicalCatalogDisplayMode: RecordSuggestionOutputMode,
    lastRecordedActivityAlias: String,
    lastRecordedDuration: String,
    collapsedCanonicalRootPaths: Set<String>,
    orderedCanonicalRootPaths: List<String>,
    suggestionsVisible: Boolean,
    isCanonicalCatalogVisible: Boolean,
    isCanonicalCatalogLoading: Boolean,
    isSuggestionsLoading: Boolean,
    isTxtPreviewVisible: Boolean,
    isTxtPreviewLoading: Boolean,
    txtPreviewStatusText: String,
    selectedMonth: String,
    selectedHistoryFile: String,
    editableHistoryContent: String,
    actualTimeExpanded: Boolean = false,
    onToggleActualTime: () -> Unit = {},
    logicalDayTarget: RecordLogicalDayTarget,
    logicalDayClock: Clock,
    onSelectLogicalDayYesterday: () -> Unit,
    onSelectLogicalDayToday: () -> Unit,
    onRefreshLogicalDayDefault: (Long) -> Unit,
    onToggleSuggestions: () -> Unit,
    onDismissSuggestions: () -> Unit,
    onSuggestedActivityClick: (String) -> Unit,
    onOpenCanonicalCatalog: () -> Unit,
    onOpenQuickAccessCanonicalCatalog: () -> Unit,
    onDismissCanonicalCatalog: () -> Unit,
    onCanonicalCatalogDisplayModeChange: (RecordSuggestionOutputMode) -> Unit,
    onCollapsedCanonicalRootPathsChange: (Set<String>) -> Unit,
    onOrderedCanonicalRootPathsChange: (List<String>) -> Unit,
    onCanonicalCatalogEntryClick: (CanonicalBrowserTarget, String) -> Boolean,
    canonicalBrowserTarget: CanonicalBrowserTarget?,
    onOpenTxtPreview: () -> Unit,
    onStartIntervalRecording: () -> Unit,
    onStopIntervalRecording: () -> Unit,
    onDiscardIntervalDraft: () -> Unit,
    onDismissTxtPreview: () -> Unit,
    onRecordNow: () -> Unit,
    onRecordInterval: () -> Unit
) {
    var currentTimeMillis by remember { mutableStateOf(System.currentTimeMillis()) }
    val currentTimeText = formatCurrentTime(currentTimeMillis)
    var quickActivitySearch by remember { mutableStateOf("") }
    val haptic = LocalHapticFeedback.current

    // Keep the displayed clock live and let the state layer auto-switch the activity-day default
    // at 06:00 whenever the user has not explicitly overridden yesterday/today on this visit.
    LaunchedEffect(Unit) {
        while (true) {
            val now = System.currentTimeMillis()
            currentTimeMillis = now
            onRefreshLogicalDayDefault(now)
            delay(1000)
        }
    }

    Column(
        modifier = Modifier.fillMaxWidth(),
        verticalArrangement = Arrangement.spacedBy(16.dp)
    ) {
        RecordTimeSettingsCard(
            currentTimeText = currentTimeText,
            logicalDayTarget = logicalDayTarget,
            actualTimeExpanded = actualTimeExpanded,
            onToggleActualTime = onToggleActualTime,
            onSelectLogicalDayYesterday = onSelectLogicalDayYesterday,
            onSelectLogicalDayToday = onSelectLogicalDayToday
        )

        RecordQuickAccessCard(
            recordContent = recordContent,
            onRecordContentChange = onRecordContentChange,
            quickActivities = quickActivities,
            availableActivityNames = availableActivityNames,
            onQuickActivitiesUpdate = onQuickActivitiesUpdate,
            quickAccessCardExpanded = quickAccessCardExpanded,
            onToggleQuickAccessCard = onToggleQuickAccessCard,
            assistSettingsExpanded = assistSettingsExpanded,
            onToggleAssistSettings = onToggleAssistSettings,
            suggestionsVisible = suggestionsVisible,
            onToggleSuggestions = onToggleSuggestions,
            onOpenQuickAccessCanonicalCatalog = onOpenQuickAccessCanonicalCatalog,
            quickActivitySearch = quickActivitySearch,
            onQuickActivitySearchChange = { quickActivitySearch = it },
            maxQuickActivityCount = MAX_QUICK_ACTIVITY_COUNT
        )

        RecordInputCard(
            authoringMode = authoringMode,
            onAuthoringModeChange = onAuthoringModeChange,
            recordContent = recordContent,
            onRecordContentChange = onRecordContentChange,
            recordRemark = recordRemark,
            onRecordRemarkChange = onRecordRemarkChange,
            intervalStart = intervalStart,
            onIntervalStartChange = onIntervalStartChange,
            intervalEnd = intervalEnd,
            onIntervalEndChange = onIntervalEndChange,
            intervalStartedAtEpochMs = intervalStartedAtEpochMs,
            attributionDateIso = attributionDateIso,
            currentTimeMillis = currentTimeMillis,
            lastRecordedActivityAlias = lastRecordedActivityAlias,
            lastRecordedDuration = lastRecordedDuration,
            onOpenCanonicalCatalog = onOpenCanonicalCatalog,
            onOpenTxtPreview = onOpenTxtPreview,
            onStartIntervalRecording = {
                haptic.performHapticFeedback(HapticFeedbackType.LongPress)
                onStartIntervalRecording()
            },
            onStopIntervalRecording = {
                haptic.performHapticFeedback(HapticFeedbackType.LongPress)
                onStopIntervalRecording()
            },
            onDiscardIntervalDraft = onDiscardIntervalDraft,
            onRecordNow = {
                haptic.performHapticFeedback(HapticFeedbackType.LongPress)
                if (authoringMode == RecordAuthoringMode.INTERVAL) {
                    onRecordInterval()
                } else {
                    onRecordNow()
                }
            }
        )
    }

    if (suggestionsVisible) {
        RecordSuggestionsSheet(
            logicalDayTarget = logicalDayTarget,
            logicalDayClock = logicalDayClock,
            suggestionLookbackDays = suggestionLookbackDays,
            suggestionTopN = suggestionTopN,
            suggestionOutputMode = suggestionOutputMode,
            isSuggestionsLoading = isSuggestionsLoading,
            suggestedActivities = suggestedActivities,
            onDismissRequest = onDismissSuggestions,
            onSuggestionLookbackDaysChange = onSuggestionLookbackDaysChange,
            onSuggestionTopNChange = onSuggestionTopNChange,
            onSuggestionOutputModeChange = onSuggestionOutputModeChange,
            onSuggestedActivityClick = onSuggestedActivityClick
        )
    }

    if (isCanonicalCatalogVisible) {
        RecordCanonicalCatalogScreen(
            isLoading = isCanonicalCatalogLoading,
            roots = canonicalCatalogRoots,
            statusText = canonicalCatalogStatusText,
            displayMode = canonicalCatalogDisplayMode,
            target = canonicalBrowserTarget,
            collapsedRootPaths = collapsedCanonicalRootPaths,
            orderedRootPaths = orderedCanonicalRootPaths,
            onDismissRequest = onDismissCanonicalCatalog,
            onDisplayModeChange = onCanonicalCatalogDisplayModeChange,
            onCollapsedRootPathsChange = onCollapsedCanonicalRootPathsChange,
            onOrderedRootPathsChange = onOrderedCanonicalRootPathsChange,
            onCanonicalPathClick = { token ->
                canonicalBrowserTarget?.let { target ->
                    if (onCanonicalCatalogEntryClick(target, token)) {
                        onDismissCanonicalCatalog()
                    }
                }
            }
        )
    }

    if (isTxtPreviewVisible) {
        RecordTxtPreviewSheet(
            txtStorageGateway = txtStorageGateway,
            selectedMonth = selectedMonth,
            selectedHistoryFile = selectedHistoryFile,
            editableHistoryContent = editableHistoryContent,
            logicalDayTarget = logicalDayTarget,
            logicalDayClock = logicalDayClock,
            isLoading = isTxtPreviewLoading,
            previewStatusText = txtPreviewStatusText,
            onDismissRequest = onDismissTxtPreview
        )
    }
}

private fun formatCurrentTime(currentTimeMillis: Long): String =
    DISPLAY_TIME_FORMATTER.format(Date(currentTimeMillis))
