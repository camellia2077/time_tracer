package com.example.tracer

import androidx.compose.animation.AnimatedVisibility
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.KeyboardArrowDown
import androidx.compose.material.icons.filled.KeyboardArrowUp
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.ModalBottomSheet
import androidx.compose.material3.SuggestionChip
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.material3.rememberModalBottomSheetState
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.record.R
import java.time.Clock

@Composable
internal fun RecordSuggestionsSection(
    suggestionsVisible: Boolean,
    isSuggestionsLoading: Boolean,
    suggestedActivities: List<String>,
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
                            SuggestionChip(
                                onClick = { onSuggestedActivityClick(activity) },
                                label = {
                                    Text(
                                        text = activity,
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

@OptIn(ExperimentalMaterial3Api::class)
@Composable
internal fun RecordSuggestionsSheet(
    logicalDayTarget: RecordLogicalDayTarget,
    logicalDayClock: Clock,
    suggestionLookbackDays: Int,
    isSuggestionsLoading: Boolean,
    suggestedActivities: List<String>,
    onDismissRequest: () -> Unit,
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
    val sheetState = rememberModalBottomSheetState(skipPartiallyExpanded = true)
    ModalBottomSheet(
        onDismissRequest = onDismissRequest,
        sheetState = sheetState
    ) {
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 24.dp, vertical = 8.dp),
            verticalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            Text(
                text = stringResource(R.string.record_suggestions_sheet_title),
                style = MaterialTheme.typography.titleLarge,
                color = MaterialTheme.colorScheme.primary
            )
            Text(
                text = stringResource(
                    R.string.record_suggestions_target_logical_day,
                    logicalDayLabel
                ),
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
            Text(
                text = stringResource(R.string.record_suggestions_target_date, targetDateIso),
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
            RecordSuggestionsSection(
                suggestionsVisible = true,
                isSuggestionsLoading = isSuggestionsLoading,
                suggestedActivities = suggestedActivities,
                loadingStateText = loadingStateText,
                emptyStateText = emptyStateText,
                onToggleSuggestions = onDismissRequest,
                onSuggestedActivityClick = {
                    onSuggestedActivityClick(it)
                    onDismissRequest()
                },
                showToggleButton = false,
                contentPadding = PaddingValues(bottom = 24.dp)
            )
        }
    }
}
