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
import androidx.compose.runtime.key
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberUpdatedState
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


@Composable
internal fun WheelNumberPicker(
    label: String,
    value: Int,
    values: IntRange,
    modifier: Modifier = Modifier,
    valueText: (Int) -> String = { "%02d".format(it) },
    onValueChange: (Int) -> Unit
) {
    val itemHeight = 48.dp
    val initialIndex = (value - values.first).coerceIn(0, values.count() - 1)
    val hapticFeedback = LocalHapticFeedback.current
    val currentValues by rememberUpdatedState(values)
    val currentOnValueChange by rememberUpdatedState(onValueChange)
    val currentHapticFeedback by rememberUpdatedState(hapticFeedback)

    // A time-bound change can alter the numeric range without changing this picker value
    // (for example, editing an interval start changes the end picker lower bound). Recreate
    // the list state for that new range so its old index never maps to a different value.
    key(values.first, values.last) {
        val listState = rememberLazyListState(initialFirstVisibleItemIndex = initialIndex)
        val flingBehavior = rememberSnapFlingBehavior(lazyListState = listState)

        LaunchedEffect(value) {
            val target = value.coerceIn(values.first, values.last) - values.first
            if (listState.firstVisibleItemIndex != target) {
                listState.animateScrollToItem(target)
            }
        }
        LaunchedEffect(listState) {
            snapshotFlow { listState.firstVisibleItemIndex }
                .drop(1)
                .collect { index ->
                    if (index in 0 until currentValues.count()) {
                        currentHapticFeedback.performHapticFeedback(HapticFeedbackType.LongPress)
                        currentOnValueChange(currentValues.elementAt(index))
                    }
                }
        }

        Column(
            modifier = modifier,
            horizontalAlignment = Alignment.CenterHorizontally
        ) {
            Text(
                text = label,
                style = MaterialTheme.typography.labelMedium,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
            LazyColumn(
                state = listState,
                flingBehavior = flingBehavior,
                modifier = Modifier
                    .fillMaxWidth()
                    .height(itemHeight * 3),
                contentPadding = PaddingValues(vertical = itemHeight),
                horizontalAlignment = androidx.compose.ui.Alignment.CenterHorizontally
            ) {
                items(values.toList()) { number ->
                    Text(
                        text = valueText(number),
                        style = MaterialTheme.typography.titleLarge,
                        modifier = Modifier
                            .alpha(
                                if (number == listState.firstVisibleItemIndex + values.first) {
                                    1f
                                } else {
                                    0.38f
                                }
                            )
                            .height(itemHeight)
                            .wrapContentHeight()
                    )
                }
            }
        }
    }
}

