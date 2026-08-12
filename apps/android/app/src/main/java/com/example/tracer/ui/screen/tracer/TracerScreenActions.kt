package com.example.tracer

import android.content.ClipData
import androidx.compose.runtime.Composable
import androidx.compose.ui.platform.ClipEntry
import androidx.compose.ui.platform.LocalClipboard
import androidx.compose.ui.res.stringResource
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

internal data class TracerScreenActions(
    val onCoordinatorEvent: (TracerCoordinatorEvent) -> Unit,
    val onCopyDiagnosticsPayload: () -> Unit,
    val onPersistRecordQuickActivities: (List<String>) -> Unit,
    val onPersistRecordQuickAccessCardExpanded: (Boolean) -> Unit,
    val onPersistRecordQuickAccessEditorVisibility: (Boolean) -> Unit,
    val onPersistRecordCanonicalCatalogDisplayMode: (RecordFrequentOutputMode) -> Unit,
    val onPersistRecordCollapsedCanonicalRootPaths: (Set<String>) -> Unit,
    val onPersistRecordOrderedCanonicalRootPaths: (List<String>) -> Unit,
    val onPersistRecordFrequentLookbackDays: (Int) -> Unit,
    val onPersistRecordFrequentOutputMode: (RecordFrequentOutputMode) -> Unit,
    val onPersistRecordFrequentTopN: (Int) -> Unit,
    val onPersistConfigCardExpanded: (com.example.tracer.data.ConfigCard, Boolean) -> Unit
)

private data class TracerScreenDiagnosticsActions(
    val onCopyDiagnosticsPayload: () -> Unit
)

private data class TracerScreenPreferenceActions(
    val onPersistRecordQuickActivities: (List<String>) -> Unit,
    val onPersistRecordQuickAccessCardExpanded: (Boolean) -> Unit,
    val onPersistRecordQuickAccessEditorVisibility: (Boolean) -> Unit,
    val onPersistRecordCanonicalCatalogDisplayMode: (RecordFrequentOutputMode) -> Unit,
    val onPersistRecordCollapsedCanonicalRootPaths: (Set<String>) -> Unit,
    val onPersistRecordOrderedCanonicalRootPaths: (List<String>) -> Unit,
    val onPersistRecordFrequentLookbackDays: (Int) -> Unit,
    val onPersistRecordFrequentOutputMode: (RecordFrequentOutputMode) -> Unit,
    val onPersistRecordFrequentTopN: (Int) -> Unit,
    val onPersistConfigCardExpanded: (com.example.tracer.data.ConfigCard, Boolean) -> Unit
)

@Composable
internal fun rememberTracerScreenActions(
    selectedTab: TracerTab,
    tabLifecycleArgs: () -> TracerTabLifecycleArgs,
    onTabChanged: (TracerTab) -> Unit,
    coroutineScope: kotlinx.coroutines.CoroutineScope,
    configGateway: ConfigGateway,
    dataViewModel: DataViewModel,
    userPreferencesRepository: com.example.tracer.data.UserPreferencesRepository,
    quickActivitiesPreferenceGateway: QuickActivitiesPreferenceGateway
): TracerScreenActions {
    val diagnosticsActions = rememberTracerScreenDiagnosticsActions(
        coroutineScope = coroutineScope,
        configGateway = configGateway,
        dataViewModel = dataViewModel
    )
    val preferenceActions = rememberTracerScreenPreferenceActions(
        coroutineScope = coroutineScope,
        userPreferencesRepository = userPreferencesRepository,
        quickActivitiesPreferenceGateway = quickActivitiesPreferenceGateway
    )
    return TracerScreenActions(
        onCoordinatorEvent = { event ->
            when (event) {
                is TracerCoordinatorEvent.SelectTab -> {
                    val nextTab = event.tab
                    if (nextTab == selectedTab) {
                        return@TracerScreenActions
                    }
                    TracerTabRegistry.onLeave(selectedTab, tabLifecycleArgs())
                    onTabChanged(nextTab)
                }
            }
        },
        onCopyDiagnosticsPayload = diagnosticsActions.onCopyDiagnosticsPayload,
        onPersistRecordQuickActivities = preferenceActions.onPersistRecordQuickActivities,
        onPersistRecordQuickAccessCardExpanded =
            preferenceActions.onPersistRecordQuickAccessCardExpanded,
        onPersistRecordQuickAccessEditorVisibility = preferenceActions.onPersistRecordQuickAccessEditorVisibility,
        onPersistRecordCanonicalCatalogDisplayMode =
            preferenceActions.onPersistRecordCanonicalCatalogDisplayMode,
        onPersistRecordCollapsedCanonicalRootPaths =
            preferenceActions.onPersistRecordCollapsedCanonicalRootPaths,
        onPersistRecordOrderedCanonicalRootPaths =
            preferenceActions.onPersistRecordOrderedCanonicalRootPaths,
        onPersistRecordFrequentLookbackDays = preferenceActions.onPersistRecordFrequentLookbackDays,
        onPersistRecordFrequentOutputMode = preferenceActions.onPersistRecordFrequentOutputMode,
        onPersistRecordFrequentTopN = preferenceActions.onPersistRecordFrequentTopN,
        onPersistConfigCardExpanded = preferenceActions.onPersistConfigCardExpanded
    )
}

@Composable
private fun rememberTracerScreenDiagnosticsActions(
    coroutineScope: kotlinx.coroutines.CoroutineScope,
    configGateway: ConfigGateway,
    dataViewModel: DataViewModel
): TracerScreenDiagnosticsActions {
    val clipboard = LocalClipboard.current
    val diagnosticsPrepareText = stringResource(R.string.tracer_diagnostics_prepare)
    return TracerScreenDiagnosticsActions(
        onCopyDiagnosticsPayload = {
            coroutineScope.launch {
                dataViewModel.setStatusText(diagnosticsPrepareText)
                val payloadResult = withContext(Dispatchers.IO) {
                    configGateway.buildDiagnosticsPayload(maxEntries = 50)
                }
                if (!payloadResult.ok || payloadResult.payload.isBlank()) {
                    dataViewModel.setStatusText(payloadResult.message)
                    return@launch
                }

                clipboard.setClipEntry(
                    ClipEntry(
                        ClipData.newPlainText(
                            "Time Tracer diagnostics",
                            payloadResult.payload
                        )
                    )
                )
                dataViewModel.setStatusText(payloadResult.message)
            }
        }
    )
}

@Composable
private fun rememberTracerScreenPreferenceActions(
    coroutineScope: kotlinx.coroutines.CoroutineScope,
    userPreferencesRepository: com.example.tracer.data.UserPreferencesRepository,
    quickActivitiesPreferenceGateway: QuickActivitiesPreferenceGateway
): TracerScreenPreferenceActions {
    return TracerScreenPreferenceActions(
        onPersistRecordQuickActivities = { activities ->
            coroutineScope.launch {
                quickActivitiesPreferenceGateway.setQuickActivities(activities)
            }
        },
        onPersistRecordQuickAccessCardExpanded = { value ->
            coroutineScope.launch {
                userPreferencesRepository.setRecordQuickAccessCardExpanded(value)
            }
        },
        onPersistRecordQuickAccessEditorVisibility = { value ->
            coroutineScope.launch {
                userPreferencesRepository.setRecordQuickAccessEditorVisible(value)
            }
        },
        onPersistRecordCanonicalCatalogDisplayMode = { value ->
            coroutineScope.launch {
                userPreferencesRepository.setRecordCanonicalCatalogDisplayMode(value)
            }
        },
        onPersistRecordCollapsedCanonicalRootPaths = { values ->
            coroutineScope.launch {
                userPreferencesRepository.setRecordCollapsedCanonicalRootPaths(values)
            }
        },
        onPersistRecordOrderedCanonicalRootPaths = { values ->
            coroutineScope.launch {
                userPreferencesRepository.setRecordOrderedCanonicalRootPaths(values)
            }
        },
        onPersistRecordFrequentLookbackDays = { value ->
            coroutineScope.launch {
                userPreferencesRepository.setRecordFrequentLookbackDays(value)
            }
        },
        onPersistRecordFrequentOutputMode = { value ->
            coroutineScope.launch {
                userPreferencesRepository.setRecordFrequentOutputMode(value)
            }
        },
        onPersistRecordFrequentTopN = { value ->
            coroutineScope.launch {
                userPreferencesRepository.setRecordFrequentTopN(value)
            }
        },
        onPersistConfigCardExpanded = { card, value ->
            coroutineScope.launch {
                userPreferencesRepository.setConfigCardExpanded(card, value)
            }
        }
    )
}
