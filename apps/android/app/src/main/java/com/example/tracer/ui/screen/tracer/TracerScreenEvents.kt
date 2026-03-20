package com.example.tracer

import androidx.compose.material3.SnackbarDuration

internal sealed interface TracerCoordinatorEvent {
    data class SelectTab(val tab: TracerTab) : TracerCoordinatorEvent
}

internal sealed interface TracerTabUiEvent {
    data class ShowSnackbar(
        val message: String,
        val duration: SnackbarDuration,
        val actionLabel: String? = null,
        val withDismissAction: Boolean = true,
        val onActionEvent: TracerCoordinatorEvent? = null
    ) : TracerTabUiEvent
}

internal data class TracerTabStatusEventArgs(
    val selectedTab: TracerTab,
    val statusText: String,
    val lastObservedTab: TracerTab,
    val lastObservedStatus: String
) {
    val isTabUnchanged: Boolean
        get() = selectedTab == lastObservedTab

    val hasStatusUpdate: Boolean
        get() = statusText.isNotBlank() && statusText != lastObservedStatus
}
