package com.example.tracer

import androidx.compose.material3.SnackbarDuration
import androidx.compose.material3.SnackbarHostState
import androidx.compose.material3.SnackbarResult
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue

internal fun statusTextForSelectedTab(
    selectedTab: TracerTab,
    dataStatusText: String,
    queryStatusText: String,
    recordStatusText: String,
    configStatusText: String
): String {
    return when (selectedTab) {
        TracerTab.DATA -> dataStatusText
        TracerTab.REPORT -> queryStatusText
        TracerTab.RECORD -> recordStatusText
        TracerTab.TXT -> recordStatusText
        TracerTab.CONFIG -> configStatusText
    }
}

@Composable
internal fun StatusSnackbarEffect(
    selectedTab: TracerTab,
    statusText: String,
    snackbarHostState: SnackbarHostState,
    onShowExportDetails: () -> Unit
) {
    var lastObservedStatus by remember { mutableStateOf(statusText) }
    var lastObservedTab by remember { mutableStateOf(selectedTab) }

    LaunchedEffect(selectedTab, statusText) {
        // TXT tab already renders inline status in the editor card.
        // Suppress global snackbar there to avoid duplicated error surface.
        if (selectedTab == TracerTab.TXT) {
            lastObservedTab = selectedTab
            lastObservedStatus = statusText
            return@LaunchedEffect
        }

        val isTabUnchanged = selectedTab == lastObservedTab
        val hasStatusUpdate = statusText.isNotBlank() && statusText != lastObservedStatus
        val isPartialExportCompletion = statusText.startsWith("Export all completed ->")
        val isFailureStatus = statusText.contains("fail", ignoreCase = true) ||
            statusText.contains("error", ignoreCase = true)
        if (isTabUnchanged && hasStatusUpdate) {
            snackbarHostState.currentSnackbarData?.dismiss()
            if (isPartialExportCompletion) {
                val result = snackbarHostState.showSnackbar(
                    message = "Export all completed with issues.",
                    actionLabel = "Details",
                    withDismissAction = true,
                    duration = SnackbarDuration.Long
                )
                if (result == SnackbarResult.ActionPerformed) {
                    onShowExportDetails()
                }
            } else {
                snackbarHostState.showSnackbar(
                    message = statusText,
                    withDismissAction = true,
                    duration = if (isFailureStatus) {
                        SnackbarDuration.Long
                    } else {
                        SnackbarDuration.Short
                    }
                )
            }
        }
        lastObservedTab = selectedTab
        lastObservedStatus = statusText
    }
}
