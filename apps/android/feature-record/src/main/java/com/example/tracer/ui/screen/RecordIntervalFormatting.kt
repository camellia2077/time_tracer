package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.layout.wrapContentHeight
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.lazy.rememberLazyListState
import androidx.compose.foundation.gestures.snapping.rememberSnapFlingBehavior
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.AccountTree
import androidx.compose.material.icons.filled.Check
import androidx.compose.material.icons.filled.Edit
import androidx.compose.material.icons.filled.Description
import androidx.compose.material.icons.filled.PlayArrow
import androidx.compose.material.icons.filled.Stop
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.ElevatedCard
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.ModalBottomSheet
import androidx.compose.material3.OutlinedCard
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.SegmentedButton
import androidx.compose.material3.SegmentedButtonDefaults
import androidx.compose.material3.SingleChoiceSegmentedButtonRow
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.focus.onFocusChanged
import androidx.compose.ui.platform.testTag
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.text.input.ImeAction
import androidx.compose.ui.unit.dp
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.runtime.getValue
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.runtime.snapshotFlow
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.hapticfeedback.HapticFeedbackType
import androidx.compose.ui.platform.LocalHapticFeedback
import kotlinx.coroutines.flow.drop
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale
import java.util.concurrent.TimeUnit
import com.example.tracer.feature.record.R
import com.example.tracer.ui.components.TracerSegmentedButtonDefaults


internal fun splitIntervalTime(value: String): List<String> {
    val digits = isoTimeDigits(value)
    return listOf(digits.substring(0, 2), digits.substring(2, 4), digits.substring(4, 6))
}

internal fun formatIsoClockTime(value: String): String {
    val digits = isoTimeDigits(value)
    return "${digits.substring(0, 2)}:${digits.substring(2, 4)}:${digits.substring(4, 6)}"
}

private fun isoTimeDigits(value: String): String =
    if (value.length == 8 && value[2] == ':' && value[5] == ':') {
        value.filterIndexed { index, _ -> index != 2 && index != 5 }
    } else {
        "000000"
    }

internal fun formatCurrentClockTime(currentTimeMillis: Long): String =
    SimpleDateFormat("HH:mm:ss", Locale.getDefault()).format(Date(currentTimeMillis))

internal fun intervalDurationSeconds(start: String, end: String): Long {
    fun toSeconds(value: String): Long {
        val digits = isoTimeDigits(value)
        return digits.substring(0, 2).toLong() * 3600L +
            digits.substring(2, 4).toLong() * 60L +
            digits.substring(4, 6).toLong()
    }

    val duration = toSeconds(end) - toSeconds(start)
    return if (duration < 0L) duration + TimeUnit.DAYS.toSeconds(1) else duration
}

internal fun intervalTimeSeconds(value: String): Long {
    val digits = isoTimeDigits(value)
    return digits.substring(0, 2).toLong() * 3600L +
        digits.substring(2, 4).toLong() * 60L +
        digits.substring(4, 6).toLong()
}

internal fun isCrossMidnightInterval(start: String, end: String): Boolean =
    intervalTimeSeconds(end) < intervalTimeSeconds(start)

internal fun formatDurationSummary(totalSeconds: Long): String {
    val normalizedSeconds = totalSeconds.coerceAtLeast(0L)
    val hours = normalizedSeconds / 3600L
    val minutes = (normalizedSeconds % 3600L) / 60L
    val seconds = normalizedSeconds % 60L
    return buildList {
        if (hours > 0L) add("${hours}h")
        if (minutes > 0L) add("${minutes}m")
        if (seconds > 0L || isEmpty()) add("${seconds}s")
    }.joinToString(" ")
}

