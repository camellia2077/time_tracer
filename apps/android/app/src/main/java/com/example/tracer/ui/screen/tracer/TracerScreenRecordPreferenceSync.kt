package com.example.tracer

import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import com.example.tracer.data.RecordSuggestionPreferences

@Composable
internal fun SyncTracerScreenRecordPreferences(
    recordSuggestionPreferences: RecordSuggestionPreferences,
    recordViewModel: RecordViewModel
) {
    LaunchedEffect(
        recordSuggestionPreferences.lookbackDays,
        recordSuggestionPreferences.topN,
        recordSuggestionPreferences.quickActivities,
        recordSuggestionPreferences.assistExpanded,
        recordSuggestionPreferences.assistSettingsExpanded
    ) {
        recordViewModel.updateSuggestionPreferences(
            lookbackDays = recordSuggestionPreferences.lookbackDays,
            topN = recordSuggestionPreferences.topN
        )
        recordViewModel.updateQuickActivities(recordSuggestionPreferences.quickActivities)
        recordViewModel.updateAssistUiState(
            assistExpanded = recordSuggestionPreferences.assistExpanded,
            assistSettingsExpanded = recordSuggestionPreferences.assistSettingsExpanded
        )
    }
}
