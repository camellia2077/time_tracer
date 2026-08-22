package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Button
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.ModalBottomSheet
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.material3.rememberModalBottomSheetState
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
import com.example.tracer.ui.components.CalendarAvailability
import kotlinx.coroutines.launch


@Composable
internal fun InsightsActivityTimeline(
    insights: StructuredDailyInsights,
    activityAggregate: ActivityAggregate,
    projectTree: List<StructuredInsightsProjectNode>,
    calendarAvailability: CalendarAvailability,
    periodComparison: InsightsPeriodComparisonState = InsightsPeriodComparisonState.Hidden,
    canComparePreviousPeriod: Boolean = false,
    comparisonColorScheme: InsightsComparisonColorScheme,
    comparisonIndicatorStyle: InsightsComparisonIndicatorStyle,
    selectedView: InsightsActivityView,
    onSelectedViewChange: (InsightsActivityView) -> Unit,
    onPeriodComparisonToggle: () -> Unit = {},
    onComparisonPeriodSelected: (InsightsPeriodSelection) -> Unit = {},
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
        InsightsActivityViewSwitcher(
            selectedView = selectedView,
            views = listOf(InsightsActivityView.RECORDS, InsightsActivityView.OVERVIEW),
            onSelect = onSelectedViewChange
        )
        Spacer(modifier = Modifier.height(8.dp))
        if (selectedView == InsightsActivityView.OVERVIEW) {
            InsightsActivityOverview(
                activityDays = listOf(insights),
                activityAggregate = activityAggregate,
                projectTree = projectTree,
                insightsMode = InsightsMode.DAY,
                periodComparison = periodComparison,
                canComparePreviousPeriod = canComparePreviousPeriod,
                comparisonColorScheme = comparisonColorScheme,
                comparisonIndicatorStyle = comparisonIndicatorStyle,
                calendarAvailability = calendarAvailability,
                onPeriodComparisonToggle = onPeriodComparisonToggle,
                onComparisonPeriodSelected = onComparisonPeriodSelected
            )
        } else {
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
            } else {
                InsightsTimelineRecordList(
                    activities = insights.activities,
                    layout = InsightsTimelineLayout.DURATION_SCALED,
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
        InsightsRemarkEditSheet(
            title = stringResource(R.string.insights_edit_activity_remark),
            subject = activity.activityName,
            remark = draftRemark,
            label = stringResource(R.string.insights_activity_remark_label),
            saveLabel = stringResource(R.string.insights_save_activity_remark),
            cancelLabel = stringResource(R.string.insights_cancel_activity_remark),
            error = editError,
            saving = saving,
            onRemarkChange = { draftRemark = it },
            onDismiss = { editingActivity = null },
            onSave = {
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
        )
    }

    if (editingDayRemark) {
        InsightsRemarkEditSheet(
            title = stringResource(R.string.insights_edit_day_remark),
            remark = dayRemarkDraft,
            label = stringResource(R.string.insights_day_remark_label),
            saveLabel = stringResource(R.string.insights_save_day_remark),
            cancelLabel = stringResource(R.string.insights_cancel_day_remark),
            error = editError,
            saving = saving,
            onRemarkChange = { dayRemarkDraft = it },
            onDismiss = { editingDayRemark = false },
            onSave = {
                saving = true
                editError = ""
                scope.launch {
                    val result = onUpdateDayRemark(dayRemarkDraft)
                    saving = false
                    if (result.ok) {
                        editingDayRemark = false
                    } else {
                        editError = result.message.ifBlank { "Day remark update failed." }
                    }
                }
            }
        )
    }
}

@Composable
@OptIn(ExperimentalMaterial3Api::class)
private fun InsightsRemarkEditSheet(
    title: String,
    remark: String,
    label: String,
    saveLabel: String,
    cancelLabel: String,
    error: String,
    saving: Boolean,
    onRemarkChange: (String) -> Unit,
    onDismiss: () -> Unit,
    onSave: () -> Unit,
    subject: String? = null
) {
    val sheetState = rememberModalBottomSheetState(skipPartiallyExpanded = true)
    ModalBottomSheet(
        onDismissRequest = { if (!saving) onDismiss() },
        sheetState = sheetState
    ) {
        Column(
            modifier = Modifier.padding(start = 24.dp, top = 8.dp, end = 24.dp, bottom = 24.dp),
            verticalArrangement = Arrangement.spacedBy(16.dp)
        ) {
            Text(text = title, style = MaterialTheme.typography.headlineSmall)
            subject?.let {
                Text(text = it, style = MaterialTheme.typography.labelMedium)
            }
            OutlinedTextField(
                value = remark,
                onValueChange = onRemarkChange,
                enabled = !saving,
                label = { Text(label) },
                modifier = Modifier.fillMaxWidth(),
                minLines = 3
            )
            if (error.isNotBlank()) {
                Text(
                    text = error,
                    color = MaterialTheme.colorScheme.error,
                    style = MaterialTheme.typography.bodySmall
                )
            }
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.End
            ) {
                TextButton(enabled = !saving, onClick = onDismiss) {
                    Text(cancelLabel)
                }
                Button(enabled = !saving, onClick = onSave) {
                    Text(saveLabel)
                }
            }
        }
    }
}
