package com.example.tracer

import androidx.compose.animation.AnimatedVisibility
import androidx.compose.foundation.BorderStroke
import androidx.compose.foundation.gestures.detectDragGesturesAfterLongPress
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Code
import androidx.compose.material.icons.filled.Close
import androidx.compose.material.icons.filled.ExpandLess
import androidx.compose.material.icons.filled.ExpandMore
import androidx.compose.material.icons.filled.KeyboardArrowDown
import androidx.compose.material.icons.filled.KeyboardArrowUp
import androidx.compose.material.icons.filled.Translate
import androidx.compose.material3.FilledIconButton
import androidx.compose.material3.HorizontalDivider
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedCard
import androidx.compose.material3.OutlinedIconButton
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Surface
import androidx.compose.material3.SuggestionChip
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.mutableStateMapOf
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.graphicsLayer
import androidx.compose.ui.hapticfeedback.HapticFeedbackType
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.layout.onGloballyPositioned
import androidx.compose.ui.layout.positionInParent
import androidx.compose.ui.platform.LocalHapticFeedback
import androidx.compose.ui.platform.testTag
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.compose.ui.window.Dialog
import androidx.compose.ui.window.DialogProperties
import androidx.compose.ui.zIndex
import com.example.tracer.feature.record.R
import java.time.Clock
import kotlin.math.abs

@Composable
internal fun RecordSuggestionsSection(
    suggestionsVisible: Boolean,
    isSuggestionsLoading: Boolean,
    suggestedActivities: List<RecordSuggestedActivity>,
    suggestionOutputMode: RecordSuggestionOutputMode,
    loadingStateText: String,
    emptyStateText: String,
    onToggleSuggestions: () -> Unit,
    onSuggestedActivityClick: (String) -> Unit,
    showToggleButton: Boolean = true,
    contentPadding: PaddingValues = PaddingValues()
) {
    Column(
        modifier = Modifier
            .fillMaxWidth()
            .padding(contentPadding)
    ) {
        if (showToggleButton) {
            TextButton(
                onClick = onToggleSuggestions,
                modifier = Modifier.fillMaxWidth()
            ) {
                Text(stringResource(R.string.record_action_suggestions))
                Spacer(Modifier.width(8.dp))
                Icon(
                    if (suggestionsVisible) {
                        Icons.Default.KeyboardArrowUp
                    } else {
                        Icons.Default.KeyboardArrowDown
                    },
                    contentDescription = null
                )
            }
        }

        AnimatedVisibility(visible = suggestionsVisible) {
            Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
                if (isSuggestionsLoading) {
                    Text(
                        loadingStateText,
                        style = MaterialTheme.typography.bodySmall
                    )
                } else if (suggestedActivities.isEmpty()) {
                    Text(
                        emptyStateText,
                        style = MaterialTheme.typography.bodySmall
                    )
                } else {
                    com.example.tracer.ui.components.SimpleFlowRow(
                        modifier = Modifier.fillMaxWidth(),
                        horizontalGap = 8.dp,
                        verticalGap = 8.dp
                    ) {
                        suggestedActivities.forEach { activity ->
                            val displayToken = activity.displayToken(suggestionOutputMode)
                            SuggestionChip(
                                onClick = { onSuggestedActivityClick(displayToken) },
                                label = {
                                    Text(
                                        text = displayToken,
                                        maxLines = 1,
                                        overflow = TextOverflow.Ellipsis
                                    )
                                }
                            )
                        }
                    }
                }
            }
        }
    }
}
@Composable
internal fun RecordSuggestionsSheet(
    logicalDayTarget: RecordLogicalDayTarget,
    logicalDayClock: Clock,
    suggestionLookbackDays: Int,
    suggestionTopN: Int,
    suggestionOutputMode: RecordSuggestionOutputMode,
    isSuggestionsLoading: Boolean,
    suggestedActivities: List<RecordSuggestedActivity>,
    onDismissRequest: () -> Unit,
    onSuggestionLookbackDaysChange: (String) -> Unit,
    onSuggestionTopNChange: (String) -> Unit,
    onSuggestionOutputModeChange: (RecordSuggestionOutputMode) -> Unit,
    onSuggestedActivityClick: (String) -> Unit
) {
    val targetDateIso = resolveLogicalDayTargetDate(
        logicalDayTarget = logicalDayTarget,
        clock = logicalDayClock
    ).toString()
    val logicalDayLabel = stringResource(
        if (logicalDayTarget == RecordLogicalDayTarget.YESTERDAY) {
            R.string.record_action_record_to_yesterday
        } else {
            R.string.record_action_record_to_today
        }
    )
    val emptyStateText = stringResource(
        R.string.record_hint_no_suggestions_lookback,
        suggestionLookbackDays
    )
    val loadingStateText = stringResource(
        R.string.record_hint_loading_suggestions_for_logical_day,
        logicalDayLabel
    )
    var settingsExpanded by remember { mutableStateOf(false) }
    var lookbackDaysInput by remember { mutableStateOf(suggestionLookbackDays.toString()) }
    var topNInput by remember { mutableStateOf(suggestionTopN.toString()) }
    Dialog(
        onDismissRequest = onDismissRequest,
        properties = DialogProperties(usePlatformDefaultWidth = false)
    ) {
        Surface(
            modifier = Modifier.fillMaxSize(),
            color = MaterialTheme.colorScheme.surface
        ) {
            Column(modifier = Modifier.fillMaxSize()) {
                Column(
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(horizontal = 24.dp, vertical = 12.dp),
                    verticalArrangement = Arrangement.spacedBy(12.dp)
                ) {
                    Row(
                        modifier = Modifier.fillMaxWidth(),
                        horizontalArrangement = Arrangement.SpaceBetween,
                        verticalAlignment = Alignment.CenterVertically
                    ) {
                        Text(
                            text = stringResource(R.string.record_suggestions_sheet_title),
                            style = MaterialTheme.typography.titleLarge,
                            color = MaterialTheme.colorScheme.primary
                        )
                        IconButton(onClick = onDismissRequest) {
                            Icon(
                                imageVector = Icons.Default.Close,
                                contentDescription = stringResource(R.string.txt_action_close)
                            )
                        }
                    }
                    Row(
                        modifier = Modifier.fillMaxWidth(),
                        horizontalArrangement = Arrangement.spacedBy(8.dp)
                    ) {
                        if (suggestionOutputMode == RecordSuggestionOutputMode.CANONICAL) {
                            FilledIconButton(onClick = {}) {
                                Icon(
                                    imageVector = Icons.Default.Code,
                                    contentDescription = stringResource(
                                        R.string.record_cd_suggestion_output_canonical_selected
                                    )
                                )
                            }
                        } else {
                            OutlinedIconButton(
                                onClick = {
                                    onSuggestionOutputModeChange(
                                        RecordSuggestionOutputMode.CANONICAL
                                    )
                                }
                            ) {
                                Icon(
                                    imageVector = Icons.Default.Code,
                                    contentDescription = stringResource(
                                        R.string.record_cd_suggestion_output_switch_to_canonical
                                    )
                                )
                            }
                        }
                        if (suggestionOutputMode == RecordSuggestionOutputMode.ALIAS) {
                            FilledIconButton(onClick = {}) {
                                Icon(
                                    imageVector = Icons.Default.Translate,
                                    contentDescription = stringResource(
                                        R.string.record_cd_suggestion_output_alias_selected
                                    )
                                )
                            }
                        } else {
                            OutlinedIconButton(
                                onClick = {
                                    onSuggestionOutputModeChange(
                                        RecordSuggestionOutputMode.ALIAS
                                    )
                                }
                            ) {
                                Icon(
                                    imageVector = Icons.Default.Translate,
                                    contentDescription = stringResource(
                                        R.string.record_cd_suggestion_output_switch_to_alias
                                    )
                                )
                            }
                        }
                    }
                    Text(
                        text = stringResource(
                            if (suggestionOutputMode == RecordSuggestionOutputMode.CANONICAL) {
                                R.string.record_suggestions_output_mode_canonical
                            } else {
                                R.string.record_suggestions_output_mode_alias
                            }
                        ),
                        style = MaterialTheme.typography.bodyMedium,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                    Row(
                        modifier = Modifier.fillMaxWidth(),
                        horizontalArrangement = Arrangement.spacedBy(16.dp)
                    ) {
                        Text(
                            text = stringResource(
                                R.string.record_suggestions_target_logical_day,
                                logicalDayLabel
                            ),
                            modifier = Modifier.weight(1f),
                            style = MaterialTheme.typography.bodyMedium,
                            color = MaterialTheme.colorScheme.onSurfaceVariant,
                            maxLines = 1,
                            overflow = TextOverflow.Ellipsis
                        )
                        Text(
                            text = stringResource(
                                R.string.record_suggestions_target_date,
                                targetDateIso
                            ),
                            modifier = Modifier.weight(1f),
                            style = MaterialTheme.typography.bodyMedium,
                            color = MaterialTheme.colorScheme.onSurfaceVariant,
                            maxLines = 1,
                            overflow = TextOverflow.Ellipsis
                        )
                    }
                    TextButton(
                        onClick = { settingsExpanded = !settingsExpanded },
                        modifier = Modifier.fillMaxWidth()
                    ) {
                        Text(stringResource(R.string.record_suggestions_settings_title))
                        Spacer(Modifier.width(8.dp))
                        Icon(
                            imageVector = if (settingsExpanded) {
                                Icons.Default.ExpandLess
                            } else {
                                Icons.Default.ExpandMore
                            },
                            contentDescription = if (settingsExpanded) {
                                stringResource(R.string.record_cd_collapse)
                            } else {
                                stringResource(R.string.record_cd_expand)
                            }
                        )
                    }
                    AnimatedVisibility(visible = settingsExpanded) {
                        Row(
                            modifier = Modifier.fillMaxWidth(),
                            horizontalArrangement = Arrangement.spacedBy(8.dp)
                        ) {
                            OutlinedTextField(
                                value = lookbackDaysInput,
                                onValueChange = { rawValue ->
                                    val digitsOnly = rawValue.filter(Char::isDigit)
                                    lookbackDaysInput = digitsOnly
                                    val parsed = digitsOnly.toIntOrNull()
                                    // `0` is a valid business input here: it means "do not query
                                    // any suggestion history yet", which also lets users clear the
                                    // field first and then type a replacement value naturally.
                                    if (parsed != null) {
                                        onSuggestionLookbackDaysChange(digitsOnly)
                                    }
                                },
                                label = { Text(stringResource(R.string.record_label_days)) },
                                modifier = Modifier.weight(1f),
                                keyboardOptions = KeyboardOptions(
                                    keyboardType = KeyboardType.Number
                                )
                            )
                            OutlinedTextField(
                                value = topNInput,
                                onValueChange = { rawValue ->
                                    val digitsOnly = rawValue.filter(Char::isDigit)
                                    topNInput = digitsOnly
                                    val parsed = digitsOnly.toIntOrNull()
                                    // `0` is a valid business input here: it means "return zero
                                    // suggestions", which keeps the query semantics aligned with
                                    // the user's ability to clear and re-enter the field.
                                    if (parsed != null) {
                                        onSuggestionTopNChange(digitsOnly)
                                    }
                                },
                                label = { Text(stringResource(R.string.record_label_top_n)) },
                                modifier = Modifier.weight(1f),
                                keyboardOptions = KeyboardOptions(
                                    keyboardType = KeyboardType.Number
                                )
                            )
                        }
                    }
                }
                HorizontalDivider()
                Box(
                    modifier = Modifier
                        .weight(1f)
                        .fillMaxWidth()
                        .verticalScroll(rememberScrollState())
                ) {
                    RecordSuggestionsSection(
                        suggestionsVisible = true,
                        isSuggestionsLoading = isSuggestionsLoading,
                        suggestedActivities = suggestedActivities,
                        suggestionOutputMode = suggestionOutputMode,
                        loadingStateText = loadingStateText,
                        emptyStateText = emptyStateText,
                        onToggleSuggestions = onDismissRequest,
                        onSuggestedActivityClick = {
                            onSuggestedActivityClick(it)
                            onDismissRequest()
                        },
                        showToggleButton = false,
                        contentPadding = PaddingValues(
                            start = 24.dp,
                            end = 24.dp,
                            top = 16.dp,
                            bottom = 12.dp
                        )
                    )
                }
            }
        }
    }
}
