package com.example.tracer

import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import com.example.tracer.PersistedRecordInputSnapshot
import com.example.tracer.data.RecordFrequentPreferences

@Composable
internal fun SyncTracerScreenRecordPreferences(
    recordFrequentPreferences: RecordFrequentPreferences,
    quickActivities: List<String>,
    persistedRecordInput: PersistedRecordInputSnapshot?,
    recordViewModel: RecordViewModel
) {
    LaunchedEffect(
        recordFrequentPreferences.lookbackDays,
        recordFrequentPreferences.topN,
        recordFrequentPreferences.outputMode,
        recordFrequentPreferences.canonicalCatalogDisplayMode,
        recordFrequentPreferences.canonicalCatalogSource,
        quickActivities,
        recordFrequentPreferences.quickAccessCardExpanded,
        recordFrequentPreferences.quickAccessEditorVisible,
        recordFrequentPreferences.collapsedCanonicalRootPaths,
        recordFrequentPreferences.orderedCanonicalRootPaths
    ) {
        recordViewModel.updateFrequentPreferencesAndReloadIfVisible(
            lookbackDays = recordFrequentPreferences.lookbackDays,
            topN = recordFrequentPreferences.topN
        )
        recordViewModel.updateFrequentOutputMode(recordFrequentPreferences.outputMode)
        recordViewModel.updateCanonicalCatalogDisplayMode(
            recordFrequentPreferences.canonicalCatalogDisplayMode
        )
        recordViewModel.updateCanonicalCatalogSource(
            recordFrequentPreferences.canonicalCatalogSource
        )
        recordViewModel.updateQuickActivities(quickActivities)
        recordViewModel.updateQuickAccessCardExpanded(
            recordFrequentPreferences.quickAccessCardExpanded
        )
        recordViewModel.updateQuickAccessEditorVisibility(
            quickAccessEditorVisible = recordFrequentPreferences.quickAccessEditorVisible
        )
        recordViewModel.updateCollapsedCanonicalRootPaths(
            recordFrequentPreferences.collapsedCanonicalRootPaths
        )
        recordViewModel.updateOrderedCanonicalRootPaths(
            recordFrequentPreferences.orderedCanonicalRootPaths
        )
    }

    LaunchedEffect(persistedRecordInput) {
        persistedRecordInput?.let { recordViewModel.hydratePersistedRecordInput(it) }
    }
}
