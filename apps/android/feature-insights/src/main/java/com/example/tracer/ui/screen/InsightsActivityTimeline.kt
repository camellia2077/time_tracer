package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.insights.R
import kotlinx.coroutines.launch


@Composable
internal fun InsightsActivityTimeline(
    insights: StructuredDailyInsights,
    modifier: Modifier = Modifier,
    onUpdateActivityRemark: suspend (ActivityTimelineItem, String) -> RecordActionResult,
    onUpdateDayRemark: suspend (String) -> RecordActionResult
) {
    val colors = insightsSemanticColors()
    var editingActivity by remember { mutableStateOf<ActivityTimelineItem?>(null) }
    var draftRemark by remember { mutableStateOf("") }
    var editError by remember { mutableStateOf("") }
    var saving by remember { mutableStateOf(false) }
    var editingDayRemark by remember { mutableStateOf(false) }
    var dayRemarkDraft by remember { mutableStateOf("") }
    val scope = rememberCoroutineScope()


    Column(modifier = modifier.fillMaxWidth()) {
        Text(
            text = stringResource(R.string.insights_result_title_activity_timeline),
            style = MaterialTheme.typography.titleMedium,
            color = colors.root
        )
        Spacer(modifier = Modifier.height(8.dp))
        insights.dayRemark.let { dayRemark ->
            Surface(
                modifier = Modifier.fillMaxWidth(),
                tonalElevation = 1.dp,
                shape = MaterialTheme.shapes.medium
            ) {
                Column(modifier = Modifier.padding(12.dp)) {
                    Text(
                        text = stringResource(R.string.insights_day_remark_label),
                        style = MaterialTheme.typography.labelMedium,
                        color = colors.child
                    )
                    Spacer(modifier = Modifier.height(4.dp))
                    Text(
                        text = dayRemark,
                        style = MaterialTheme.typography.bodyMedium,
                        color = MaterialTheme.colorScheme.onSurface
                    )
                    TextButton(onClick = {
                        dayRemarkDraft = dayRemark
                        editError = ""
                        editingDayRemark = true
                    }) {
                        Text(stringResource(R.string.insights_edit_day_remark))
                    }
                }
            }
            Spacer(modifier = Modifier.height(8.dp))
        }
        if (insights.activities.isEmpty()) {
            Text(
                text = stringResource(R.string.insights_activity_timeline_empty),
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
            return
        }

        Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
            insights.activities
                .map(ActivityTimelineItem::toInsightsTimelineEntry)
                .forEach { entry ->
                    InsightsTimelineEntryRow(
                        entry = entry,
                        colors = colors,
                        onEditRemark = { activity ->
                            editingActivity = activity
                            draftRemark = activity.remark.orEmpty()
                            editError = ""
                        }
                    )
                }
        }
    }

    editingActivity?.let { activity ->
        AlertDialog(
            onDismissRequest = { if (!saving) editingActivity = null },
            title = { Text(stringResource(R.string.insights_edit_activity_remark)) },
            text = {
                Column {
                    Text(
                        text = activity.activityName,
                        style = MaterialTheme.typography.labelMedium
                    )
                    Spacer(modifier = Modifier.height(8.dp))
                    OutlinedTextField(
                        value = draftRemark,
                        onValueChange = { draftRemark = it },
                        enabled = !saving,
                        label = { Text(stringResource(R.string.insights_activity_remark_label)) },
                        modifier = Modifier.fillMaxWidth()
                    )
                    if (editError.isNotBlank()) {
                        Spacer(modifier = Modifier.height(8.dp))
                        Text(
                            text = editError,
                            color = MaterialTheme.colorScheme.error,
                            style = MaterialTheme.typography.bodySmall
                        )
                    }
                }
            },
            confirmButton = {
                TextButton(
                    enabled = !saving,
                    onClick = {
                        saving = true
                        editError = ""
                        scope.launch {
                            val result = onUpdateActivityRemark(activity, draftRemark)
                            saving = false
                            if (result.ok) {
                                editingActivity = null
                            } else {
                                editError = result.message.ifBlank {
                                    "Activity remark update failed."
                                }
                            }
                        }
                    }
                ) {
                    Text(stringResource(R.string.insights_save_activity_remark))
                }
            },
            dismissButton = {
                TextButton(
                    enabled = !saving,
                    onClick = { editingActivity = null }
                ) {
                    Text(stringResource(R.string.insights_cancel_activity_remark))
                }
            }
        )
    }

    if (editingDayRemark) {
        AlertDialog(
            onDismissRequest = { if (!saving) editingDayRemark = false },
            title = { Text(stringResource(R.string.insights_edit_day_remark)) },
            text = {
                Column {
                    OutlinedTextField(
                        value = dayRemarkDraft,
                        onValueChange = { dayRemarkDraft = it },
                        enabled = !saving,
                        label = { Text(stringResource(R.string.insights_day_remark_label)) },
                        modifier = Modifier.fillMaxWidth()
                    )
                    if (editError.isNotBlank()) {
                        Spacer(modifier = Modifier.height(8.dp))
                        Text(editError, color = MaterialTheme.colorScheme.error)
                    }
                }
            },
            confirmButton = {
                TextButton(enabled = !saving, onClick = {
                    saving = true
                    editError = ""
                    scope.launch {
                        val result = onUpdateDayRemark(dayRemarkDraft)
                        saving = false
                        if (result.ok) editingDayRemark = false
                        else editError = result.message.ifBlank { "Day remark update failed." }
                    }
                }) { Text(stringResource(R.string.insights_save_day_remark)) }
            },
            dismissButton = {
                TextButton(enabled = !saving, onClick = { editingDayRemark = false }) {
                    Text(stringResource(R.string.insights_cancel_day_remark))
                }
            }
        )
    }
}
