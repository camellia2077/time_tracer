package com.example.tracer

import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import com.example.tracer.PersistedRecordInputSnapshot
import com.example.tracer.data.RecordSuggestionPreferences

@Composable
internal fun SyncTracerScreenRecordPreferences(
    recordSuggestionPreferences: RecordSuggestionPreferences,
    quickActivities: List<String>,
    persistedRecordInput: PersistedRecordInputSnapshot?,
    recordViewModel: RecordViewModel
) {
    LaunchedEffect(
        recordSuggestionPreferences.lookbackDays,
        recordSuggestionPreferences.topN,
        recordSuggestionPreferences.outputMode,
        recordSuggestionPreferences.canonicalCatalogDisplayMode,
        quickActivities,
        recordSuggestionPreferences.quickAccessCardExpanded,
        recordSuggestionPreferences.assistSettingsExpanded,
        recordSuggestionPreferences.collapsedCanonicalRootPaths,
        recordSuggestionPreferences.orderedCanonicalRootPaths
    ) {
        recordViewModel.updateSuggestionPreferences(
            lookbackDays = recordSuggestionPreferences.lookbackDays,
            topN = recordSuggestionPreferences.topN
        )
        recordViewModel.updateSuggestionOutputMode(recordSuggestionPreferences.outputMode)
        recordViewModel.updateCanonicalCatalogDisplayMode(
            recordSuggestionPreferences.canonicalCatalogDisplayMode
        )
        recordViewModel.updateQuickActivities(quickActivities)
        recordViewModel.updateQuickAccessCardExpanded(
            recordSuggestionPreferences.quickAccessCardExpanded
        )
        recordViewModel.updateAssistUiState(
            assistSettingsExpanded = recordSuggestionPreferences.assistSettingsExpanded
        )
        recordViewModel.updateCollapsedCanonicalRootPaths(
            recordSuggestionPreferences.collapsedCanonicalRootPaths
        )
        recordViewModel.updateOrderedCanonicalRootPaths(
            recordSuggestionPreferences.orderedCanonicalRootPaths
        )
    }

    LaunchedEffect(persistedRecordInput) {
        persistedRecordInput?.let { recordViewModel.hydratePersistedRecordInput(it) }
    }
}
