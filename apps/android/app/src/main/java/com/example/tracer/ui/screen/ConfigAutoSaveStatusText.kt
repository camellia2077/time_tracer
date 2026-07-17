package com.example.tracer

import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource

@Composable
internal fun ConfigAutoSaveStatusText(
    autoSaveStatus: ConfigAutoSaveStatus,
    modifier: Modifier = Modifier
) {
    val (text, color) = when (autoSaveStatus) {
        ConfigAutoSaveStatus.IDLE -> {
            stringResource(R.string.config_auto_save_status_idle) to
                MaterialTheme.colorScheme.onSurfaceVariant
        }
        ConfigAutoSaveStatus.SAVING -> {
            stringResource(R.string.config_auto_save_status_saving) to
                MaterialTheme.colorScheme.onSurfaceVariant
        }
        ConfigAutoSaveStatus.SAVED -> {
            stringResource(R.string.config_auto_save_status_saved) to
                MaterialTheme.colorScheme.primary
        }
        ConfigAutoSaveStatus.FAILED -> {
            stringResource(R.string.config_auto_save_status_failed) to
                MaterialTheme.colorScheme.error
        }
    }
    Text(
        text = text,
        modifier = modifier,
        style = MaterialTheme.typography.bodySmall,
        color = color
    )
}
