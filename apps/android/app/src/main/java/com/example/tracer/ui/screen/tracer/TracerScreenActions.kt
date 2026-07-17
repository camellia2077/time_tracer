package com.example.tracer

import androidx.compose.runtime.Composable
import androidx.compose.ui.platform.LocalClipboardManager
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.AnnotatedString
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

internal data class TracerScreenActions(
    val onCoordinatorEvent: (TracerCoordinatorEvent) -> Unit,
    val onCopyDiagnosticsPayload: () -> Unit,
    val onPersistRecordQuickActivities: (List<String>) -> Unit,
    val onPersistRecordAssistSettingsExpanded: (Boolean) -> Unit,
    val onPersistRecordCanonicalCatalogDisplayMode: (RecordSuggestionOutputMode) -> Unit,
    val onPersistRecordCollapsedCanonicalRootPaths: (Set<String>) -> Unit,
    val onPersistRecordOrderedCanonicalRootPaths: (List<String>) -> Unit,
    val onPersistRecordSuggestLookbackDays: (Int) -> Unit,
    val onPersistRecordSuggestOutputMode: (RecordSuggestionOutputMode) -> Unit,
    val onPersistRecordSuggestTopN: (Int) -> Unit
)

private data class TracerScreenDiagnosticsActions(
    val onCopyDiagnosticsPayload: () -> Unit
)

private data class TracerScreenPreferenceActions(
    val onPersistRecordQuickActivities: (List<String>) -> Unit,
    val onPersistRecordAssistSettingsExpanded: (Boolean) -> Unit,
    val onPersistRecordCanonicalCatalogDisplayMode: (RecordSuggestionOutputMode) -> Unit,
    val onPersistRecordCollapsedCanonicalRootPaths: (Set<String>) -> Unit,
    val onPersistRecordOrderedCanonicalRootPaths: (List<String>) -> Unit,
    val onPersistRecordSuggestLookbackDays: (Int) -> Unit,
    val onPersistRecordSuggestOutputMode: (RecordSuggestionOutputMode) -> Unit,
    val onPersistRecordSuggestTopN: (Int) -> Unit
)

@Composable
internal fun rememberTracerScreenActions(
    selectedTab: TracerTab,
    tabLifecycleArgs: () -> TracerTabLifecycleArgs,
    onTabChanged: (TracerTab) -> Unit,
    coroutineScope: kotlinx.coroutines.CoroutineScope,
    configGateway: ConfigGateway,
    configViewModel: ConfigViewModel,
    userPreferencesRepository: com.example.tracer.data.UserPreferencesRepository
): TracerScreenActions {
    val diagnosticsActions = rememberTracerScreenDiagnosticsActions(
        coroutineScope = coroutineScope,
        configGateway = configGateway,
        configViewModel = configViewModel
    )
    val preferenceActions = rememberTracerScreenPreferenceActions(
        coroutineScope = coroutineScope,
        userPreferencesRepository = userPreferencesRepository
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
        onPersistRecordAssistSettingsExpanded = preferenceActions.onPersistRecordAssistSettingsExpanded,
        onPersistRecordCanonicalCatalogDisplayMode =
            preferenceActions.onPersistRecordCanonicalCatalogDisplayMode,
        onPersistRecordCollapsedCanonicalRootPaths =
            preferenceActions.onPersistRecordCollapsedCanonicalRootPaths,
        onPersistRecordOrderedCanonicalRootPaths =
            preferenceActions.onPersistRecordOrderedCanonicalRootPaths,
        onPersistRecordSuggestLookbackDays = preferenceActions.onPersistRecordSuggestLookbackDays,
        onPersistRecordSuggestOutputMode = preferenceActions.onPersistRecordSuggestOutputMode,
        onPersistRecordSuggestTopN = preferenceActions.onPersistRecordSuggestTopN
    )
}

@Composable
private fun rememberTracerScreenDiagnosticsActions(
    coroutineScope: kotlinx.coroutines.CoroutineScope,
    configGateway: ConfigGateway,
    configViewModel: ConfigViewModel
): TracerScreenDiagnosticsActions {
    val clipboardManager = LocalClipboardManager.current
    val diagnosticsPrepareText = stringResource(R.string.tracer_diagnostics_prepare)
    return TracerScreenDiagnosticsActions(
        onCopyDiagnosticsPayload = {
            coroutineScope.launch {
                configViewModel.setStatusText(diagnosticsPrepareText)
                val payloadResult = withContext(Dispatchers.IO) {
                    configGateway.buildDiagnosticsPayload(maxEntries = 50)
                }
                if (!payloadResult.ok || payloadResult.payload.isBlank()) {
                    configViewModel.setStatusText(payloadResult.message)
                    return@launch
                }

                clipboardManager.setText(AnnotatedString(payloadResult.payload))
                configViewModel.setStatusText(payloadResult.message)
            }
        }
    )
}

@Composable
private fun rememberTracerScreenPreferenceActions(
    coroutineScope: kotlinx.coroutines.CoroutineScope,
    userPreferencesRepository: com.example.tracer.data.UserPreferencesRepository
): TracerScreenPreferenceActions {
    return TracerScreenPreferenceActions(
        onPersistRecordQuickActivities = { activities ->
            coroutineScope.launch {
                userPreferencesRepository.setRecordQuickActivities(activities)
            }
        },
        onPersistRecordAssistSettingsExpanded = { value ->
            coroutineScope.launch {
                userPreferencesRepository.setRecordAssistSettingsExpanded(value)
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
        onPersistRecordSuggestLookbackDays = { value ->
            coroutineScope.launch {
                userPreferencesRepository.setRecordSuggestLookbackDays(value)
            }
        },
        onPersistRecordSuggestOutputMode = { value ->
            coroutineScope.launch {
                userPreferencesRepository.setRecordSuggestOutputMode(value)
            }
        },
        onPersistRecordSuggestTopN = { value ->
            coroutineScope.launch {
                userPreferencesRepository.setRecordSuggestTopN(value)
            }
        }
    )
}
