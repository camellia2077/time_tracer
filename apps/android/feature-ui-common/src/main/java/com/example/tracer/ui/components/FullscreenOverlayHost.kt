package com.example.tracer.ui.components

import androidx.compose.runtime.Composable
import androidx.compose.runtime.compositionLocalOf
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue

class FullscreenOverlayHost {
    var content: (@Composable () -> Unit)? by mutableStateOf(null)
        private set

    fun show(content: @Composable () -> Unit) {
        this.content = content
    }

    fun dismiss() {
        content = null
    }
}

val LocalFullscreenOverlayHost = compositionLocalOf<FullscreenOverlayHost?> { null }
