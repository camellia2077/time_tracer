package com.example.tracer

import androidx.compose.runtime.Composable
import androidx.compose.runtime.remember
import androidx.compose.ui.platform.LocalConfiguration
import androidx.compose.ui.res.stringResource
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.launch
import java.util.Locale

@Composable
internal fun RecordTabContent(
    recordUiState: RecordUiState,
    recordViewModel: RecordViewModel,
    validMappingNames: Set<String>,
    userPreferencesRepository: com.example.tracer.data.UserPreferencesRepository,
    coroutineScope: CoroutineScope
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
        stringResource(R.string.tracer_status_quick_activities_save_failed_empty_validation)
    val quickActivitiesCannotBeEmptyText =
        stringResource(R.string.tracer_status_quick_activities_cannot_be_empty)
    val quickActivitiesExceedLimitTemplate =
        stringResource(R.string.tracer_status_quick_activities_exceed_limit)
    val invalidQuickActivitiesTemplate =
        stringResource(R.string.tracer_status_invalid_quick_activities)
    val quickActivitiesSavedTemplate =
        stringResource(R.string.tracer_status_quick_activities_saved)
    val invalidSuggestedActivityTemplate =
        stringResource(R.string.tracer_status_invalid_suggested_activity)

    RecordSection(
        recordContent = recordUiState.recordContent,
        onRecordContentChange = recordViewModel::onRecordContentChange,
        recordRemark = recordUiState.recordRemark,
        onRecordRemarkChange = recordViewModel::onRecordRemarkChange,
        quickActivities = recordUiState.quickActivities,
        availableActivityNames = remember(validMappingNames) { validMappingNames.toList().sorted() },
        onQuickActivitiesUpdate = { targetActivities ->
            if (validMappingNames.isEmpty()) {
                recordViewModel.setStatusText(
                    quickActivitiesSaveFailedEmptyValidationText
                )
                return@RecordSection
            }
            val normalized = targetActivities
                .map { it.trim() }
                .filter { it.isNotEmpty() }
                .distinct()
            if (normalized.isEmpty()) {
                recordViewModel.setStatusText(
                    quickActivitiesCannotBeEmptyText
                )
                return@RecordSection
            }
            if (normalized.size > 12) {
                recordViewModel.setStatusText(
                    formatWithLocale(locale, quickActivitiesExceedLimitTemplate, 12)
                )
                return@RecordSection
            }
            val invalidActivities = normalized.filter { !validMappingNames.contains(it) }
            if (invalidActivities.isNotEmpty()) {
                recordViewModel.setStatusText(
                    formatWithLocale(
                        locale,
                        invalidQuickActivitiesTemplate,
                        invalidActivities.joinToString(", ")
                    )
                )
                return@RecordSection
            }
            recordViewModel.updateQuickActivities(normalized)
            recordViewModel.setStatusText(
                formatWithLocale(locale, quickActivitiesSavedTemplate, normalized.size)
            )
            coroutineScope.launch {
                userPreferencesRepository.setRecordQuickActivities(normalized)
            }
        },
        assistExpanded = recordUiState.assistExpanded,
        assistSettingsExpanded = recordUiState.assistSettingsExpanded,
        onToggleAssist = {
            val nextValue = !recordUiState.assistExpanded
            recordViewModel.updateAssistUiState(
                assistExpanded = nextValue,
                assistSettingsExpanded = recordUiState.assistSettingsExpanded
            )
            coroutineScope.launch {
                userPreferencesRepository.setRecordAssistExpanded(nextValue)
            }
        },
        onToggleAssistSettings = {
            val nextValue = !recordUiState.assistSettingsExpanded
            recordViewModel.updateAssistUiState(
                assistExpanded = recordUiState.assistExpanded,
                assistSettingsExpanded = nextValue
            )
            coroutineScope.launch {
                userPreferencesRepository.setRecordAssistSettingsExpanded(nextValue)
            }
        },
        suggestionLookbackDays = recordUiState.suggestionLookbackDays,
        suggestionTopN = recordUiState.suggestionTopN,
        onSuggestionLookbackDaysChange = { rawValue ->
            val parsed = rawValue.trim().toIntOrNull()
            if (parsed == null || parsed <= 0) {
                return@RecordSection
            }
            recordViewModel.updateSuggestionPreferences(
                lookbackDays = parsed,
                topN = recordUiState.suggestionTopN
            )
            coroutineScope.launch {
                userPreferencesRepository.setRecordSuggestLookbackDays(parsed)
            }
        },
        onSuggestionTopNChange = { rawValue ->
            val parsed = rawValue.trim().toIntOrNull()
            if (parsed == null || parsed <= 0) {
                return@RecordSection
            }
            recordViewModel.updateSuggestionPreferences(
                lookbackDays = recordUiState.suggestionLookbackDays,
                topN = parsed
            )
            coroutineScope.launch {
                userPreferencesRepository.setRecordSuggestTopN(parsed)
            }
        },
        suggestedActivities = recordUiState.suggestedActivities,
        suggestionsVisible = recordUiState.suggestionsVisible,
        isSuggestionsLoading = recordUiState.isSuggestionsLoading,
        useManualDate = recordUiState.useManualDate,
        manualDate = recordUiState.manualDate,
        onUseAutoDate = recordViewModel::useAutoDate,
        onUseManualDate = recordViewModel::useManualDate,
        onManualDateChange = recordViewModel::onManualDateChange,
        onToggleSuggestions = recordViewModel::toggleSuggestions,
        onSuggestedActivityClick = { activity ->
            if (validMappingNames.isNotEmpty() && !validMappingNames.contains(activity)) {
                recordViewModel.setStatusText(
                    formatWithLocale(
                        locale,
                        invalidSuggestedActivityTemplate,
                        activity
                    )
                )
                return@RecordSection
            }
            recordViewModel.applySuggestedActivity(activity)
        },
        onRecordNow = recordViewModel::recordNow
    )
}

private fun formatWithLocale(locale: Locale, template: String, vararg args: Any): String =
    String.format(locale, template, *args)
