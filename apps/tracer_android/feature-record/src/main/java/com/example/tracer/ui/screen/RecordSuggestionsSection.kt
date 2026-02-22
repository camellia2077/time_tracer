package com.example.tracer

import androidx.compose.animation.AnimatedVisibility
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.width
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.KeyboardArrowDown
import androidx.compose.material.icons.filled.KeyboardArrowUp
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.SuggestionChip
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.record.R

@Composable
internal fun RecordSuggestionsSection(
    suggestionsVisible: Boolean,
    isSuggestionsLoading: Boolean,
    suggestedActivities: List<String>,
    onToggleSuggestions: () -> Unit,
    onSuggestedActivityClick: (String) -> Unit
) {
    Column(modifier = Modifier.fillMaxWidth()) {
        TextButton(
            onClick = onToggleSuggestions,
            modifier = Modifier.fillMaxWidth()
        ) {
            Text(
                if (suggestionsVisible) {
                    stringResource(R.string.record_action_hide_suggestions)
                } else {
                    stringResource(R.string.record_action_show_suggestions)
                }
            )
            Spacer(Modifier.width(8.dp))
            Icon(
                if (suggestionsVisible) Icons.Default.KeyboardArrowUp else Icons.Default.KeyboardArrowDown,
                contentDescription = null
            )
        }

        AnimatedVisibility(visible = suggestionsVisible) {
            Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
                if (isSuggestionsLoading) {
                    Text(
                        stringResource(R.string.record_hint_loading),
                        style = MaterialTheme.typography.bodySmall
                    )
                } else if (suggestedActivities.isEmpty()) {
                    Text(
                        stringResource(R.string.record_hint_no_suggestions),
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
