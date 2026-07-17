package com.example.tracer

import androidx.compose.material3.SnackbarDuration
import androidx.compose.material3.SnackbarVisuals

internal sealed interface TracerCoordinatorEvent {
    data class SelectTab(val tab: TracerTab) : TracerCoordinatorEvent
}

// Most tabs emit a single snackbar message, but Record success needs a stable
// "title + detail" layout. Keeping that shape in visuals lets the host render
// two text rows without depending on newline parsing inside Material components.
internal data class TracerSnackbarVisuals(
    override val message: String,
    val supportingText: String? = null,
    override val actionLabel: String? = null,
    override val withDismissAction: Boolean = true,
    override val duration: SnackbarDuration = SnackbarDuration.Short
) : SnackbarVisuals

internal sealed interface TracerTabUiEvent {
    data class ShowSnackbar(
        val visuals: TracerSnackbarVisuals,
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
