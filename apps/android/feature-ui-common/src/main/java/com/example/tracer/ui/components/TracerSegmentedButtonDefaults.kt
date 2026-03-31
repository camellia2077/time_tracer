package com.example.tracer.ui.components

import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.SegmentedButtonDefaults
import androidx.compose.runtime.Composable
import androidx.compose.ui.text.font.FontWeight

object TracerSegmentedButtonDefaults {
    val activeLabelFontWeight: FontWeight = FontWeight.SemiBold
    val inactiveLabelFontWeight: FontWeight = FontWeight.Medium

    @Composable
    fun colors() = SegmentedButtonDefaults.colors(
        activeContainerColor = MaterialTheme.colorScheme.primaryContainer,
        activeContentColor = MaterialTheme.colorScheme.onPrimaryContainer,
        inactiveContainerColor = MaterialTheme.colorScheme.surface,
        inactiveContentColor = MaterialTheme.colorScheme.onSurfaceVariant,
        activeBorderColor = MaterialTheme.colorScheme.primary,
        inactiveBorderColor = MaterialTheme.colorScheme.outlineVariant
    )
}
