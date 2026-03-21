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
    val onPersistRecordAssistExpanded: (Boolean) -> Unit,
    val onPersistRecordAssistSettingsExpanded: (Boolean) -> Unit,
    val onPersistRecordSuggestLookbackDays: (Int) -> Unit,
    val onPersistRecordSuggestTopN: (Int) -> Unit
)

private data class TracerScreenDiagnosticsActions(
    val onCopyDiagnosticsPayload: () -> Unit
)

private data class TracerScreenPreferenceActions(
    val onPersistRecordQuickActivities: (List<String>) -> Unit,
    val onPersistRecordAssistExpanded: (Boolean) -> Unit,
    val onPersistRecordAssistSettingsExpanded: (Boolean) -> Unit,
    val onPersistRecordSuggestLookbackDays: (Int) -> Unit,
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
        onPersistRecordAssistExpanded = preferenceActions.onPersistRecordAssistExpanded,
        onPersistRecordAssistSettingsExpanded = preferenceActions.onPersistRecordAssistSettingsExpanded,
        onPersistRecordSuggestLookbackDays = preferenceActions.onPersistRecordSuggestLookbackDays,
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
        onPersistRecordAssistExpanded = { value ->
            coroutineScope.launch {
                userPreferencesRepository.setRecordAssistExpanded(value)
            }
        },
        onPersistRecordAssistSettingsExpanded = { value ->
            coroutineScope.launch {
                userPreferencesRepository.setRecordAssistSettingsExpanded(value)
            }
        },
        onPersistRecordSuggestLookbackDays = { value ->
            coroutineScope.launch {
                userPreferencesRepository.setRecordSuggestLookbackDays(value)
            }
        },
        onPersistRecordSuggestTopN = { value ->
            coroutineScope.launch {
                userPreferencesRepository.setRecordSuggestTopN(value)
            }
        }
    )
}
