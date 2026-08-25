package com.example.tracer

import android.app.Activity
import android.content.Context
import android.content.ContextWrapper
import android.view.View
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.navigationBarsPadding
import androidx.compose.foundation.layout.statusBarsPadding
import androidx.compose.foundation.layout.widthIn
import androidx.compose.material3.Button
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.DisposableEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.luminance
import androidx.compose.ui.graphics.toArgb
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.platform.LocalView
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.record.R
import com.example.tracer.ui.components.CalendarDatePickerSheet
import com.example.tracer.ui.components.NativeMultilineTextEditor
import com.example.tracer.ui.components.NativeMultilineTextEditorController
import com.example.tracer.ui.components.filterDigits
import java.time.DayOfWeek
import java.time.LocalDate
import java.time.YearMonth
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch


@Composable
internal fun TxtRawEditorFullScreen(
    outputMode: TxtOutputMode,
    selectedMonth: String,
    selectedDay: LocalDate?,
    value: String,
    hasUnsavedChanges: Boolean,
    canSave: Boolean,
    readOnly: Boolean,
    onValueChange: (String) -> Unit,
    onSave: () -> Unit,
    onDiscard: () -> Unit
) {
    var discardConfirmationVisible by remember { mutableStateOf(false) }
    var datePickerVisible by remember { mutableStateOf(false) }
    var jumpStatusText by remember { mutableStateOf("") }
    val editorController = remember { NativeMultilineTextEditorController() }
    val coroutineScope = rememberCoroutineScope()
    val context = LocalContext.current
    val rawMonth = remember(selectedMonth) {
        runCatching { YearMonth.parse(selectedMonth) }.getOrNull()
    }
    fun requestDiscard() {
        if (hasUnsavedChanges) {
            discardConfirmationVisible = true
        } else {
            onDiscard()
        }
    }

    val surfaceColor = MaterialTheme.colorScheme.surface
    RawEditorSystemBars(surfaceColor)
    androidx.compose.material3.Surface(modifier = Modifier.fillMaxSize(), color = surfaceColor) {
        Column(
            modifier = Modifier
                .fillMaxSize()
                .statusBarsPadding()
                .navigationBarsPadding()
                .padding(20.dp),
            verticalArrangement = Arrangement.spacedBy(12.dp)
        ) {
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Column(modifier = Modifier.weight(1f)) {
                        Text(
                            text = stringResource(
                                if (outputMode == TxtOutputMode.DAY) {
                                    R.string.txt_raw_editor_day_title
                                } else {
                                    R.string.txt_raw_editor_month_title
                                }
                            ),
                            style = MaterialTheme.typography.headlineSmall
                        )
                        Text(
                            text = if (hasUnsavedChanges) {
                                stringResource(R.string.txt_status_unsaved)
                            } else {
                                stringResource(R.string.txt_status_saved)
                            },
                            style = MaterialTheme.typography.bodySmall,
                            color = MaterialTheme.colorScheme.onSurfaceVariant
                        )
                    }
                    if (outputMode == TxtOutputMode.ALL && rawMonth != null) {
                        TextButton(onClick = { datePickerVisible = true }) {
                            Text(stringResource(R.string.txt_raw_editor_jump_to_date))
                        }
                    }
                    TextButton(onClick = ::requestDiscard) {
                        Text(stringResource(R.string.txt_action_close))
                    }
                }
                if (jumpStatusText.isNotBlank()) {
                    Text(
                        text = jumpStatusText,
                        style = MaterialTheme.typography.bodySmall,
                        color = MaterialTheme.colorScheme.error
                    )
                }
                NativeMultilineTextEditor(
                    value = value,
                    onValueChange = onValueChange,
                    modifier = Modifier
                        .fillMaxWidth()
                        .weight(1f),
                    minLines = 24,
                    monospace = true,
                    controller = editorController,
                    readOnly = readOnly
                )
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.End,
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    TextButton(onClick = ::requestDiscard) {
                        Text(stringResource(R.string.txt_raw_editor_discard))
                    }
                    Spacer(modifier = Modifier.widthIn(min = 8.dp))
                    Button(onClick = onSave, enabled = canSave) {
                        Text(stringResource(R.string.txt_cd_ingest))
                    }
                }
        }
    }
    if (datePickerVisible && rawMonth != null) {
        CalendarDatePickerSheet(
            displayMonth = rawMonth,
            selectedDate = selectedDay?.takeIf { YearMonth.from(it) == rawMonth },
            onDateSelected = { date ->
                datePickerVisible = false
                val markerOffset = findRawMonthDayMarkerOffset(value, date)
                if (markerOffset < 0) {
                    jumpStatusText = context.getString(
                        R.string.txt_raw_editor_day_not_found,
                        date.toString()
                    )
                } else {
                    jumpStatusText = ""
                    val markerEnd = markerOffset + 5
                    editorController.requestSelection(markerOffset, markerEnd)
                    coroutineScope.launch {
                        delay(800)
                        editorController.requestSelection(markerOffset)
                    }
                }
            },
            onDismissRequest = { datePickerVisible = false },
            allowAdjacentMonthSelection = false,
            firstDayOfWeek = DayOfWeek.MONDAY
        )
    }
    if (discardConfirmationVisible) {
        AlertDialog(
            onDismissRequest = { discardConfirmationVisible = false },
            title = { Text(stringResource(R.string.txt_raw_editor_discard_confirm_title)) },
            text = { Text(stringResource(R.string.txt_raw_editor_discard_confirm_message)) },
            confirmButton = {
                TextButton(onClick = {
                    discardConfirmationVisible = false
                    onDiscard()
                }) {
                    Text(stringResource(R.string.txt_raw_editor_discard))
                }
            },
            dismissButton = {
                TextButton(onClick = { discardConfirmationVisible = false }) {
                    Text(stringResource(R.string.txt_action_close))
                }
            }
        )
    }
}

@Composable
private fun RawEditorSystemBars(color: Color) {
    val view = LocalView.current
    DisposableEffect(view, color) {
        val activity = view.context.findActivity()
        val window = activity?.window
        if (window == null) {
            onDispose {}
        } else {
            val decorView = window.decorView
            val originalStatusBarColor = window.statusBarColor
            val originalNavigationBarColor = window.navigationBarColor
            val originalSystemUiVisibility = decorView.systemUiVisibility
            val lightSystemBars = color.luminance() > 0.5f
            window.statusBarColor = color.toArgb()
            window.navigationBarColor = color.toArgb()
            decorView.systemUiVisibility = if (lightSystemBars) {
                originalSystemUiVisibility or
                    View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR or
                    View.SYSTEM_UI_FLAG_LIGHT_NAVIGATION_BAR
            } else {
                originalSystemUiVisibility and
                    View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR.inv() and
                    View.SYSTEM_UI_FLAG_LIGHT_NAVIGATION_BAR.inv()
            }
            onDispose {
                window.statusBarColor = originalStatusBarColor
                window.navigationBarColor = originalNavigationBarColor
                decorView.systemUiVisibility = originalSystemUiVisibility
            }
        }
    }
}

private tailrec fun Context.findActivity(): Activity? = when (this) {
    is Activity -> this
    is ContextWrapper -> baseContext.findActivity()
    else -> null
}

internal fun splitDayMarkerDigits(value: String): Pair<String, String> {
    val digits = filterDigits(value, 4)
    return Pair(digits.take(2), digits.drop(2).take(2))
}

internal fun findRawMonthDayMarkerOffset(value: String, date: LocalDate): Int {
    val marker = "d${formatDayMarker(date)}"
    return Regex("(?m)^${Regex.escape(marker)}\\r?$")
        .find(value)
        ?.range
        ?.first
        ?: -1
}

@Composable
internal fun formatEditorCurrentDayText(date: LocalDate): String {
    val weekdayLabel = stringResource(
        when (date.dayOfWeek) {
            DayOfWeek.MONDAY -> R.string.txt_weekday_mon
            DayOfWeek.TUESDAY -> R.string.txt_weekday_tue
            DayOfWeek.WEDNESDAY -> R.string.txt_weekday_wed
            DayOfWeek.THURSDAY -> R.string.txt_weekday_thu
            DayOfWeek.FRIDAY -> R.string.txt_weekday_fri
            DayOfWeek.SATURDAY -> R.string.txt_weekday_sat
            DayOfWeek.SUNDAY -> R.string.txt_weekday_sun
        }
    )
    return "$date $weekdayLabel"
}
