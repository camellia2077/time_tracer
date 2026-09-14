package com.example.tracer.ui.components

import androidx.activity.compose.BackHandler
import androidx.compose.foundation.layout.WindowInsetsSides
import androidx.compose.foundation.layout.only
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.consumeWindowInsets
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.imePadding
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.text.BasicTextField
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.ArrowBack
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.material3.TopAppBar
import androidx.compose.material3.TopAppBarDefaults
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.remember
import androidx.compose.ui.Modifier
import androidx.compose.ui.focus.FocusRequester
import androidx.compose.ui.focus.focusRequester
import androidx.compose.ui.graphics.SolidColor
import androidx.compose.ui.text.TextStyle
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.compose.ui.window.Dialog
import androidx.compose.ui.window.DialogProperties
import com.example.tracer.fullscreenScrollContentPadding
import com.example.tracer.fullscreenContentWindowInsets

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun FullscreenTextEditor(
    title: String,
    label: String,
    text: String,
    showLabel: Boolean = true,
    emptyTextHint: String? = label,
    closeContentDescription: String,
    saving: Boolean,
    error: String,
    onTextChange: (String) -> Unit,
    onDismiss: () -> Unit
) {
    val focusRequester = remember { FocusRequester() }
    val scrollState = rememberScrollState()

    LaunchedEffect(Unit) {
        if (text.isBlank()) {
            focusRequester.requestFocus()
        }
    }

    Dialog(
        onDismissRequest = onDismiss,
        properties = DialogProperties(
            dismissOnClickOutside = false,
            usePlatformDefaultWidth = false,
            decorFitsSystemWindows = false
        )
    ) {
        Scaffold(
            modifier = Modifier.fillMaxSize(),
            contentWindowInsets = fullscreenContentWindowInsets().only(
                WindowInsetsSides.Top + WindowInsetsSides.Horizontal
            ),
            containerColor = MaterialTheme.colorScheme.surface,
            topBar = {
                TopAppBar(
                    title = { Text(title) },
                    colors = TopAppBarDefaults.topAppBarColors(
                        containerColor = MaterialTheme.colorScheme.surface
                    ),
                    navigationIcon = {
                        androidx.compose.material3.IconButton(
                            onClick = onDismiss,
                            enabled = !saving
                        ) {
                            androidx.compose.material3.Icon(
                                imageVector = Icons.AutoMirrored.Filled.ArrowBack,
                                contentDescription = closeContentDescription
                            )
                        }
                    },
                    actions = {
                        if (saving) {
                            CircularProgressIndicator(
                                modifier = Modifier.padding(horizontal = 20.dp),
                                strokeWidth = 2.dp
                            )
                        }
                    }
                )
            }
        ) { innerPadding ->
            Column(
                modifier = Modifier
                    .fillMaxSize()
                    .padding(innerPadding)
                    .consumeWindowInsets(innerPadding)
                    .imePadding()
                    .padding(start = 20.dp, end = 20.dp, top = 16.dp)
            ) {
                if (showLabel) {
                    Text(
                        text = label,
                        style = MaterialTheme.typography.labelLarge,
                        color = MaterialTheme.colorScheme.primary
                    )
                }
                BasicTextField(
                    value = text,
                    onValueChange = onTextChange,
                    enabled = !saving,
                    textStyle = MaterialTheme.typography.bodyMedium.copy(
                        color = MaterialTheme.colorScheme.onSurface,
                        lineHeight = 22.sp
                    ),
                    cursorBrush = SolidColor(MaterialTheme.colorScheme.primary),
                    modifier = Modifier
                        .fillMaxWidth()
                        .weight(1f)
                        .padding(top = 12.dp)
                        .verticalScroll(scrollState)
                        .fullscreenScrollContentPadding()
                        .padding(bottom = 16.dp)
                        .focusRequester(focusRequester),
                    decorationBox = { innerTextField ->
                        if (text.isBlank() && !emptyTextHint.isNullOrBlank()) {
                            Text(
                                text = emptyTextHint,
                                style = TextStyle(
                                    color = MaterialTheme.colorScheme.onSurfaceVariant,
                                    fontSize = 14.sp,
                                    lineHeight = 22.sp
                                )
                            )
                        }
                        innerTextField()
                    }
                )
                if (error.isNotBlank()) {
                    Text(
                        text = error,
                        color = MaterialTheme.colorScheme.error,
                        style = MaterialTheme.typography.bodySmall,
                        modifier = Modifier.fullscreenScrollContentPadding().padding(top = 12.dp, bottom = 16.dp)
                    )
                }
            }
        }
    }

    BackHandler(enabled = !saving) {
        onDismiss()
    }
}
