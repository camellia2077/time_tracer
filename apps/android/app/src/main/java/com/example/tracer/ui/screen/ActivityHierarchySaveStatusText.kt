package com.example.tracer

import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.width
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.CheckCircle
import androidx.compose.material.icons.filled.Error
import androidx.compose.material.icons.filled.Info
import androidx.compose.material.icons.filled.Sync
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.Alignment
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp

@Composable
internal fun ActivityHierarchySaveStatusText(
    autoSaveStatus: ActivityHierarchySaveStatus,
    modifier: Modifier = Modifier
) {
    val (text, icon, color) = when (autoSaveStatus) {
        ActivityHierarchySaveStatus.IDLE -> {
            Triple(
                stringResource(R.string.config_auto_save_status_idle),
                Icons.Filled.Info,
                MaterialTheme.colorScheme.onSurfaceVariant
            )
        }
        ActivityHierarchySaveStatus.SAVING -> {
            Triple(
                stringResource(R.string.config_auto_save_status_saving),
                Icons.Filled.Sync,
                MaterialTheme.colorScheme.onSurfaceVariant
            )
        }
        ActivityHierarchySaveStatus.SAVED -> {
            Triple(
                stringResource(R.string.config_auto_save_status_saved),
                Icons.Filled.CheckCircle,
                MaterialTheme.colorScheme.primary
            )
        }
        ActivityHierarchySaveStatus.FAILED -> {
            Triple(
                stringResource(R.string.config_auto_save_status_failed),
                Icons.Filled.Error,
                MaterialTheme.colorScheme.error
            )
        }
    }
    Row(
        modifier = modifier,
        verticalAlignment = Alignment.CenterVertically
    ) {
        Icon(imageVector = icon, contentDescription = null, tint = color)
        Spacer(modifier = Modifier.width(8.dp))
        Text(
            text = text,
            style = MaterialTheme.typography.labelMedium,
            color = color
        )
    }
}
