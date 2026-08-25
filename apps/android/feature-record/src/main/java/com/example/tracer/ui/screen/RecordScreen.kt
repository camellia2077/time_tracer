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
    quickAccessEditorVisible: Boolean,
    onToggleQuickAccessEditor: () -> Unit,
    frequentLookbackDays: Int,
    frequentTopN: Int,
    onFrequentLookbackDaysChange: (String) -> Unit,
    onFrequentTopNChange: (String) -> Unit,
    frequentOutputMode: RecordFrequentOutputMode,
    onFrequentOutputModeChange: (RecordFrequentOutputMode) -> Unit,
    frequentActivities: List<RecordFrequentActivity>,
    canonicalCatalogRoots: List<CanonicalPathNode>,
    canonicalCatalogStatusText: String,
    canonicalCatalogDisplayMode: RecordFrequentOutputMode,
    lastRecordedActivityHierarchyLeaf: String,
    lastRecordedDuration: String,
    latestActivityRecord: LatestActivityRecord? = null,
    previousActivityTail: PreviousActivityTail? = null,
    collapsedCanonicalRootPaths: Set<String>,
    orderedCanonicalRootPaths: List<String>,
    frequentActivitiesVisible: Boolean,
    isCanonicalCatalogVisible: Boolean,
    isCanonicalCatalogLoading: Boolean,
    isFrequentActivitiesLoading: Boolean,
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
    onToggleFrequentActivities: () -> Unit,
    onDismissFrequentActivities: () -> Unit,
    onQuickAccessFrequentActivityClick: (String) -> Unit,
    onFrequentActivitiesRequested: () -> Unit,
    onFrequentActivityClick: (String) -> Boolean,
    onTreeRequested: () -> Unit,
    categoriesContent: @Composable () -> Unit,
    onOpenCanonicalCatalog: () -> Unit,
    onOpenQuickAccessCanonicalCatalog: () -> Unit,
    onDismissCanonicalCatalog: () -> Unit,
    onCanonicalCatalogDisplayModeChange: (RecordFrequentOutputMode) -> Unit,
    canonicalCatalogSource: CanonicalCatalogSource,
    onCanonicalCatalogSourceChange: (CanonicalCatalogSource) -> Unit,
    onCollapsedCanonicalRootPathsChange: (Set<String>) -> Unit,
    onOrderedCanonicalRootPathsChange: (List<String>) -> Unit,
    onCanonicalCatalogEntryClick: (CanonicalBrowserTarget, CanonicalCatalogEntry) -> Boolean,
    canonicalBrowserTarget: CanonicalBrowserTarget?,
    onOpenTxtPreview: () -> Unit,
    onStartIntervalRecording: () -> Unit,
    onStopIntervalRecording: () -> Unit,
    onDiscardIntervalDraft: () -> Unit,
    onUsePreviousActivityEndTime: () -> Unit = {},
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
            quickAccessEditorVisible = quickAccessEditorVisible,
            onToggleQuickAccessEditor = onToggleQuickAccessEditor,
            frequentActivitiesVisible = frequentActivitiesVisible,
            onToggleFrequentActivities = onToggleFrequentActivities,
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
            lastRecordedActivityHierarchyLeaf = lastRecordedActivityHierarchyLeaf,
            lastRecordedDuration = lastRecordedDuration,
            latestActivityRecord = latestActivityRecord,
            previousActivityTail = previousActivityTail,
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
            onUsePreviousActivityEndTime = onUsePreviousActivityEndTime,
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

    if (frequentActivitiesVisible) {
        RecordFrequentActivitiesSheet(
            logicalDayTarget = logicalDayTarget,
            logicalDayClock = logicalDayClock,
            frequentLookbackDays = frequentLookbackDays,
            frequentTopN = frequentTopN,
            frequentOutputMode = frequentOutputMode,
            isFrequentActivitiesLoading = isFrequentActivitiesLoading,
            frequentActivities = frequentActivities,
            onDismissRequest = onDismissFrequentActivities,
            onFrequentLookbackDaysChange = onFrequentLookbackDaysChange,
            onFrequentTopNChange = onFrequentTopNChange,
            onFrequentOutputModeChange = onFrequentOutputModeChange,
            onFrequentActivityClick = onQuickAccessFrequentActivityClick
        )
    }

    if (isCanonicalCatalogVisible) {
        val onEntrySelected: (CanonicalCatalogEntry) -> Unit = { entry ->
            canonicalBrowserTarget?.let { target ->
                if (onCanonicalCatalogEntryClick(target, entry)) {
                    onDismissCanonicalCatalog()
                }
            }
        }
        if (canonicalBrowserTarget == CanonicalBrowserTarget.RECORD_INPUT) {
            RecordCanonicalCatalogScreen(
                isLoading = isCanonicalCatalogLoading,
                roots = canonicalCatalogRoots,
                statusText = canonicalCatalogStatusText,
                displayMode = canonicalCatalogDisplayMode,
                source = canonicalCatalogSource,
                onSourceChange = onCanonicalCatalogSourceChange,
                isFrequentActivitiesLoading = isFrequentActivitiesLoading,
                frequentActivities = frequentActivities,
                frequentLookbackDays = frequentLookbackDays,
                frequentTopN = frequentTopN,
                onFrequentActivitiesRequested = onFrequentActivitiesRequested,
                onFrequentLookbackDaysChange = onFrequentLookbackDaysChange,
                onFrequentTopNChange = onFrequentTopNChange,
                onFrequentActivityClick = onFrequentActivityClick,
                onTreeRequested = onTreeRequested,
                categoriesContent = categoriesContent,
                collapsedRootPaths = collapsedCanonicalRootPaths,
                orderedRootPaths = orderedCanonicalRootPaths,
                onDismissRequest = onDismissCanonicalCatalog,
                onDisplayModeChange = onCanonicalCatalogDisplayModeChange,
                onCollapsedRootPathsChange = onCollapsedCanonicalRootPathsChange,
                onOrderedRootPathsChange = onOrderedCanonicalRootPathsChange,
                onCanonicalEntryClick = onEntrySelected
            )
        } else {
            CanonicalActivityPickerScreen(
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
                onCanonicalEntryClick = onEntrySelected
            )
        }
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
