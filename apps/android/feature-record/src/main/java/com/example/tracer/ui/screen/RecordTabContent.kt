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
    validAuthorableEventTokens: Set<String>,
    onPersistQuickActivities: (List<String>) -> Unit,
    onPersistAssistExpanded: (Boolean) -> Unit,
    onPersistAssistSettingsExpanded: (Boolean) -> Unit,
    onPersistSuggestionLookbackDays: (Int) -> Unit,
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
    val invalidSuggestedActivityTemplate =
        stringResource(R.string.record_status_invalid_suggested_activity)

    RecordSection(
        recordContent = recordUiState.recordContent,
        onRecordContentChange = recordViewModel::onRecordContentChange,
        recordRemark = recordUiState.recordRemark,
        onRecordRemarkChange = recordViewModel::onRecordRemarkChange,
        quickActivities = recordUiState.quickActivities,
        availableActivityNames = remember(validAuthorableEventTokens) {
            validAuthorableEventTokens.toList().sorted()
        },
        onQuickActivitiesUpdate = { targetActivities ->
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
                recordViewModel.setStatusText(
                    quickActivitiesSaveFailedEmptyValidationText
                )
                return@RecordSection false
            }
            if (normalized.size > 12) {
                recordViewModel.setStatusText(
                    quickActivitiesExceedLimitText
                )
                return@RecordSection false
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
                return@RecordSection false
            }
            if (normalized == currentNormalized) {
                return@RecordSection false
            }
            recordViewModel.updateQuickActivities(normalized)
            recordViewModel.setStatusText(
                formatWithLocale(locale, quickActivitiesSavedTemplate, normalized.size)
            )
            onPersistQuickActivities(normalized)
            true
        },
        assistExpanded = recordUiState.assistExpanded,
        assistSettingsExpanded = recordUiState.assistSettingsExpanded,
        onToggleAssist = {
            val nextValue = !recordUiState.assistExpanded
            recordViewModel.updateAssistUiState(
                assistExpanded = nextValue,
                assistSettingsExpanded = recordUiState.assistSettingsExpanded
            )
            onPersistAssistExpanded(nextValue)
        },
        onToggleAssistSettings = {
            val nextValue = !recordUiState.assistSettingsExpanded
            recordViewModel.updateAssistUiState(
                assistExpanded = recordUiState.assistExpanded,
                assistSettingsExpanded = nextValue
            )
            onPersistAssistSettingsExpanded(nextValue)
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
            onPersistSuggestionLookbackDays(parsed)
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
            onPersistSuggestionTopN(parsed)
        },
        suggestedActivities = recordUiState.suggestedActivities,
        suggestionsVisible = recordUiState.suggestionsVisible,
        isSuggestionsLoading = recordUiState.isSuggestionsLoading,
        logicalDayTarget = recordUiState.logicalDayTarget,
        onSelectLogicalDayYesterday = recordViewModel::selectLogicalDayYesterday,
        onSelectLogicalDayToday = recordViewModel::selectLogicalDayToday,
        onRefreshLogicalDayDefault = recordViewModel::refreshLogicalDayDefault,
        onToggleSuggestions = recordViewModel::toggleSuggestions,
        onSuggestedActivityClick = { activity ->
            if (validAuthorableEventTokens.isNotEmpty() &&
                !validAuthorableEventTokens.contains(activity)
            ) {
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
