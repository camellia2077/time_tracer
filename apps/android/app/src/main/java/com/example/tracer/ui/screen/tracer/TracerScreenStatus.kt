package com.example.tracer

import androidx.compose.material3.SnackbarHostState
import androidx.compose.material3.SnackbarResult
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue

@Composable
internal fun StatusSnackbarEffect(
    selectedTab: TracerTab,
    statusText: String,
    snackbarHostState: SnackbarHostState,
    onCoordinatorEvent: (TracerCoordinatorEvent) -> Unit
) {
    var lastObservedStatus by remember { mutableStateOf(statusText) }
    var lastObservedTab by remember { mutableStateOf(selectedTab) }

    LaunchedEffect(selectedTab, statusText) {
        val event = TracerTabRegistry.statusEvent(
            tab = selectedTab,
            args = TracerTabStatusEventArgs(
                selectedTab = selectedTab,
                statusText = statusText,
                lastObservedTab = lastObservedTab,
                lastObservedStatus = lastObservedStatus
            )
        )
        when (event) {
            is TracerTabUiEvent.ShowSnackbar -> {
                snackbarHostState.currentSnackbarData?.dismiss()
                val result = snackbarHostState.showSnackbar(
                    message = event.message,
                    actionLabel = event.actionLabel,
                    withDismissAction = event.withDismissAction,
                    duration = event.duration
                )
                if (result == SnackbarResult.ActionPerformed && event.onActionEvent != null) {
                    onCoordinatorEvent(event.onActionEvent)
                }
            }

            null -> Unit
        }
        lastObservedTab = selectedTab
        lastObservedStatus = statusText
    }
}
