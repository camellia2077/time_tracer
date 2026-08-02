package com.example.tracer

import androidx.compose.runtime.Composable
import androidx.compose.runtime.remember
import androidx.compose.ui.platform.LocalConfiguration
import androidx.compose.ui.res.pluralStringResource
import androidx.compose.ui.res.stringResource
import com.example.tracer.feature.record.R
import java.util.Locale

@Composable
fun RecordTabContent(
    recordUiState: RecordUiState,
    recordViewModel: RecordViewModel,
    txtStorageGateway: TxtStorageGateway,
    validAuthorableEventTokens: Set<String>,
    onPersistQuickActivities: (List<String>) -> Unit,
    onPersistQuickAccessCardExpanded: (Boolean) -> Unit,
    onPersistAssistSettingsExpanded: (Boolean) -> Unit,
    onPersistCanonicalCatalogDisplayMode: (RecordSuggestionOutputMode) -> Unit,
    onPersistCollapsedCanonicalRootPaths: (Set<String>) -> Unit,
    onPersistOrderedCanonicalRootPaths: (List<String>) -> Unit,
    onPersistSuggestionLookbackDays: (Int) -> Unit,
    onPersistSuggestionOutputMode: (RecordSuggestionOutputMode) -> Unit,
    onPersistSuggestionTopN: (Int) -> Unit
) {
    val configuration = LocalConfiguration.current
    val locale = remember(configuration) {
        if (configuration.locales.isEmpty) {
            Locale.getDefault()
        } else {
            configuration.locales[0]
        }
    }
    val quickActivitiesSaveFailedEmptyValidationText =
        stringResource(R.string.record_status_quick_activities_save_failed_empty_validation)
    val quickActivitiesExceedLimitText = pluralStringResource(
        id = R.plurals.record_status_quick_activities_exceed_limit,
        count = 12,
        12
    )
    val invalidQuickActivitiesTemplate =
        stringResource(R.string.record_status_invalid_quick_activities)
    val quickActivitiesSavedTemplate =
        stringResource(R.string.record_status_quick_activities_saved)

    fun updateQuickActivities(targetActivities: List<String>): Boolean {
        val currentNormalized = recordUiState.quickActivities
            .map { it.trim() }
            .filter { it.isNotEmpty() }
            .distinct()
        val normalized = targetActivities
            .map { it.trim() }
            .filter { it.isNotEmpty() }
            .distinct()
        val isRemovalOnlyUpdate = normalized.size <= currentNormalized.size &&
            normalized.all(currentNormalized::contains)

        // Deleting quick activities should never depend on authorable-token validation.
        // This lets users clear shipped defaults before re-adding entries that match config.
        if (!isRemovalOnlyUpdate && validAuthorableEventTokens.isEmpty()) {
            recordViewModel.setStatusText(quickActivitiesSaveFailedEmptyValidationText)
            return false
        }
        if (normalized.size > 12) {
            recordViewModel.setStatusText(quickActivitiesExceedLimitText)
            return false
        }
        val invalidActivities = normalized.filter {
            !validAuthorableEventTokens.contains(it)
        }
        if (!isRemovalOnlyUpdate && invalidActivities.isNotEmpty()) {
            recordViewModel.setStatusText(
                formatWithLocale(
                    locale,
                    invalidQuickActivitiesTemplate,
                    invalidActivities.joinToString(", ")
                )
            )
            return false
        }
        if (normalized == currentNormalized) {
            return false
        }
        recordViewModel.updateQuickActivities(normalized)
        recordViewModel.setStatusText(
            formatWithLocale(locale, quickActivitiesSavedTemplate, normalized.size)
        )
        onPersistQuickActivities(normalized)
        return true
    }

    RecordSection(
        txtStorageGateway = txtStorageGateway,
        authoringMode = recordUiState.authoringMode,
        onAuthoringModeChange = recordViewModel::onAuthoringModeChange,
        recordContent = recordUiState.recordContent,
        onRecordContentChange = recordViewModel::onRecordContentChange,
        recordRemark = recordUiState.recordRemark,
        onRecordRemarkChange = recordViewModel::onRecordRemarkChange,
        intervalStart = recordUiState.intervalStart,
        onIntervalStartChange = recordViewModel::onIntervalStartChange,
        intervalEnd = recordUiState.intervalEnd,
        onIntervalEndChange = recordViewModel::onIntervalEndChange,
        intervalStartedAtEpochMs = recordUiState.intervalStartedAtEpochMs,
        attributionDateIso = recordUiState.attributionDateIso,
        quickActivities = recordUiState.quickActivities,
        availableActivityNames = remember(validAuthorableEventTokens) {
            validAuthorableEventTokens.toList().sorted()
        },
        onQuickActivitiesUpdate = ::updateQuickActivities,
        quickAccessCardExpanded = recordUiState.quickAccessCardExpanded,
        onToggleQuickAccessCard = {
            val nextExpanded = !recordUiState.quickAccessCardExpanded
            recordViewModel.updateQuickAccessCardExpanded(nextExpanded)
            onPersistQuickAccessCardExpanded(nextExpanded)
            if (!nextExpanded && recordUiState.assistSettingsExpanded) {
                recordViewModel.updateAssistUiState(assistSettingsExpanded = false)
                onPersistAssistSettingsExpanded(false)
            }
        },
        assistSettingsExpanded = recordUiState.assistSettingsExpanded,
        onToggleAssistSettings = {
            val nextValue = !recordUiState.assistSettingsExpanded
            recordViewModel.updateAssistUiState(
                assistSettingsExpanded = nextValue
            )
            if (!recordUiState.quickAccessCardExpanded) {
                recordViewModel.updateQuickAccessCardExpanded(true)
            }
            onPersistAssistSettingsExpanded(nextValue)
        },
        suggestionLookbackDays = recordUiState.suggestionLookbackDays,
        suggestionTopN = recordUiState.suggestionTopN,
        onSuggestionLookbackDaysChange = { rawValue ->
            val parsed = rawValue.trim().toIntOrNull()
            if (parsed == null || parsed < 0) {
                return@RecordSection
            }
            recordViewModel.updateSuggestionPreferencesAndReloadIfVisible(
                lookbackDays = parsed,
                topN = recordUiState.suggestionTopN
            )
            onPersistSuggestionLookbackDays(parsed)
        },
        onSuggestionTopNChange = { rawValue ->
            val parsed = rawValue.trim().toIntOrNull()
            if (parsed == null || parsed < 0) {
                return@RecordSection
            }
            recordViewModel.updateSuggestionPreferencesAndReloadIfVisible(
                lookbackDays = recordUiState.suggestionLookbackDays,
                topN = parsed
            )
            onPersistSuggestionTopN(parsed)
        },
        suggestionOutputMode = recordUiState.suggestionOutputMode,
        onSuggestionOutputModeChange = { mode ->
            recordViewModel.updateSuggestionOutputMode(mode)
            onPersistSuggestionOutputMode(mode)
        },
        suggestedActivities = recordUiState.suggestedActivities,
        canonicalCatalogRoots = recordUiState.canonicalCatalogRoots,
        canonicalCatalogStatusText = recordUiState.canonicalCatalogStatusText,
        canonicalCatalogDisplayMode = recordUiState.canonicalCatalogDisplayMode,
        lastRecordedActivityAlias = recordUiState.lastRecordedActivityAlias,
        lastRecordedDuration = recordUiState.lastRecordedDuration,
        collapsedCanonicalRootPaths = recordUiState.collapsedCanonicalRootPaths,
        orderedCanonicalRootPaths = recordUiState.orderedCanonicalRootPaths,
        suggestionsVisible = recordUiState.suggestionsVisible,
        isCanonicalCatalogVisible = recordUiState.isCanonicalCatalogVisible,
        canonicalBrowserTarget = recordUiState.canonicalBrowserTarget,
        isCanonicalCatalogLoading = recordUiState.isCanonicalCatalogLoading,
        isSuggestionsLoading = recordUiState.isSuggestionsLoading,
        isTxtPreviewVisible = recordUiState.isTxtPreviewVisible,
        isTxtPreviewLoading = recordUiState.isTxtPreviewLoading,
        txtPreviewStatusText = recordUiState.txtPreviewStatusText,
        selectedMonth = recordUiState.selectedMonth,
        selectedHistoryFile = recordUiState.selectedHistoryFile,
        editableHistoryContent = recordUiState.editableHistoryContent,
        actualTimeExpanded = recordUiState.actualTimeExpanded,
        onToggleActualTime = {
            recordViewModel.updateActualTimeExpanded(!recordUiState.actualTimeExpanded)
        },
        logicalDayTarget = recordUiState.logicalDayTarget,
        logicalDayClock = recordViewModel.logicalDayClock,
        onSelectLogicalDayYesterday = recordViewModel::selectLogicalDayYesterday,
        onSelectLogicalDayToday = recordViewModel::selectLogicalDayToday,
        onRefreshLogicalDayDefault = recordViewModel::refreshLogicalDayDefault,
        onToggleSuggestions = recordViewModel::toggleSuggestions,
        onDismissSuggestions = recordViewModel::dismissSuggestions,
        onSuggestedActivityClick = { activity ->
            val alias = recordUiState.suggestedActivities
                .firstOrNull { suggestion ->
                    suggestion.canonicalToken == activity || suggestion.aliasToken == activity
                }
                ?.aliasToken
                ?.trim()
                ?.takeIf { it.isNotEmpty() }
            if (alias != null && updateQuickActivities(recordUiState.quickActivities + alias)) {
                recordViewModel.dismissSuggestions()
            }
        },
        onOpenCanonicalCatalog = recordViewModel::openCanonicalCatalog,
        onOpenQuickAccessCanonicalCatalog = recordViewModel::openQuickAccessCanonicalCatalog,
        onDismissCanonicalCatalog = recordViewModel::dismissCanonicalCatalog,
        onCanonicalCatalogDisplayModeChange = { mode ->
            recordViewModel.updateCanonicalCatalogDisplayMode(mode)
            onPersistCanonicalCatalogDisplayMode(mode)
        },
        onCollapsedCanonicalRootPathsChange = { paths ->
            recordViewModel.updateCollapsedCanonicalRootPaths(paths)
            onPersistCollapsedCanonicalRootPaths(paths)
        },
        onOrderedCanonicalRootPathsChange = { paths ->
            recordViewModel.updateOrderedCanonicalRootPaths(paths)
            onPersistOrderedCanonicalRootPaths(paths)
        },
        onCanonicalCatalogEntryClick = { target, entry ->
            when (target) {
                CanonicalBrowserTarget.RECORD_INPUT -> {
                    val displayToken = if (
                        recordUiState.canonicalCatalogDisplayMode == RecordSuggestionOutputMode.ALIAS
                    ) {
                        entry.aliases.firstOrNull { it.isNotBlank() }?.trim()
                            ?: entry.canonicalPath.trim()
                    } else {
                        entry.canonicalPath.trim()
                    }
                    recordViewModel.applyCanonicalCatalogEntry(
                        displayToken
                    )
                    true
                }

                CanonicalBrowserTarget.QUICK_ACCESS -> {
                    val alias = entry.aliases.firstOrNull { it.isNotBlank() }?.trim()
                    if (alias == null) {
                        recordViewModel.setStatusText(
                            quickActivitiesSaveFailedEmptyValidationText
                        )
                        false
                    } else {
                        updateQuickActivities(recordUiState.quickActivities + alias)
                    }
                }

                CanonicalBrowserTarget.REPORT_STATUS_PARENT -> false
            }
        },
        onOpenTxtPreview = recordViewModel::openTxtPreview,
        onStartIntervalRecording = recordViewModel::startIntervalRecording,
        onStopIntervalRecording = recordViewModel::stopIntervalRecording,
        onDiscardIntervalDraft = recordViewModel::discardIntervalDraft,
        onDismissTxtPreview = recordViewModel::dismissTxtPreview,
        onRecordNow = recordViewModel::recordNow,
        onRecordInterval = recordViewModel::recordInterval
    )
}

private fun formatWithLocale(locale: Locale, template: String, vararg args: Any): String =
    String.format(locale, template, *args)
