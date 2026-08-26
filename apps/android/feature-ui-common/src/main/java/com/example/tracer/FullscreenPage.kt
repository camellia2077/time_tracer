package com.example.tracer

import androidx.activity.compose.BackHandler
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.WindowInsets
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.safeDrawing
import androidx.compose.foundation.layout.windowInsetsPadding
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.runtime.Composable
import androidx.compose.runtime.DisposableEffect
import androidx.compose.runtime.State
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateListOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberUpdatedState
import androidx.compose.runtime.staticCompositionLocalOf
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color

private val LocalFullscreenPageHost = staticCompositionLocalOf<FullscreenPageHostState?> { null }

private class FullscreenPageEntry(
    val onDismissRequest: State<() -> Unit>,
    val content: State<@Composable () -> Unit>
)

private class FullscreenPageHostState {
    private val pages = mutableStateListOf<FullscreenPageEntry>()

    val activePages: List<FullscreenPageEntry>
        get() = pages

    fun register(entry: FullscreenPageEntry) {
        if (entry !in pages) {
            pages += entry
        }
    }

    fun unregister(entry: FullscreenPageEntry) {
        pages.remove(entry)
    }
}

/**
 * Hosts application pages that visually cover the entire activity, including the system-bar area.
 *
 * Feature code registers pages through [FullscreenPage]; this host deliberately lives at the app
 * shell so these pages never create their own Dialog window.
 */
@Composable
fun FullscreenPageHost(content: @Composable () -> Unit) {
    val hostState = remember { FullscreenPageHostState() }
    androidx.compose.runtime.CompositionLocalProvider(
        LocalFullscreenPageHost provides hostState
    ) {
        Box(modifier = Modifier.fillMaxSize()) {
            content()
            hostState.activePages.forEach { page ->
                // Compose the page's default dismissal before its content so a page can
                // deliberately handle Back for a more specific in-page navigation state.
                BackHandler(onBack = page.onDismissRequest.value)
                page.content.value()
            }
        }
    }
}

/**
 * Registers a full-screen route with [FullscreenPageHost].
 *
 * When used outside the application shell (for example in a preview), it still renders as a
 * full-size surface, while production screens are always rendered by the shell host above.
 */
@Composable
fun FullscreenPage(
    onDismissRequest: () -> Unit,
    backgroundColor: Color = MaterialTheme.colorScheme.surface,
    content: @Composable () -> Unit
) {
    val hostState = LocalFullscreenPageHost.current
    if (hostState == null) {
        FullscreenPageContainer(backgroundColor, content)
        return
    }

    val latestDismissRequest = rememberUpdatedState(onDismissRequest)
    val latestContent = rememberUpdatedState<@Composable () -> Unit> {
        FullscreenPageContainer(backgroundColor, content)
    }
    val entry = remember {
        FullscreenPageEntry(
            onDismissRequest = latestDismissRequest,
            content = latestContent
        )
    }
    DisposableEffect(hostState, entry) {
        hostState.register(entry)
        onDispose { hostState.unregister(entry) }
    }
}

@Composable
private fun FullscreenPageContainer(
    backgroundColor: Color = MaterialTheme.colorScheme.surface,
    content: @Composable () -> Unit
) {
    Surface(
        modifier = Modifier.fillMaxSize(),
        color = backgroundColor
    ) {
        Box(
            modifier = Modifier
                .fillMaxSize()
                .windowInsetsPadding(WindowInsets.safeDrawing)
        ) {
            content()
        }
    }
}
