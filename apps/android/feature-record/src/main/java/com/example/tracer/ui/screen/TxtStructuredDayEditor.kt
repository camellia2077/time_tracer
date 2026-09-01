package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Close
import androidx.compose.material.icons.outlined.Delete
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.Button
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.ModalBottomSheet
import androidx.compose.material3.OutlinedCard
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.PrimaryTabRow
import androidx.compose.material3.Tab
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.material3.rememberModalBottomSheetState
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.pluralStringResource
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.record.R
import com.example.tracer.ui.components.formatDisplayClockTime
import kotlinx.coroutines.launch

@Composable
internal fun TxtStructuredDayEditor(
    result: TxtDayEditResolveResult,
    use12HourTime: Boolean = false,
    roots: List<CanonicalPathNode>,
    catalogLoading: Boolean,
    catalogStatusText: String,
    collapsedRootPaths: Set<String>,
    orderedRootPaths: List<String>,
    onCollapsedRootPathsChange: (Set<String>) -> Unit,
    onOrderedRootPathsChange: (List<String>) -> Unit,
    onApply: (dayRemark: String, events: List<TxtDayEditEvent>) -> Unit
) {
    var dayRemark by remember(result.normalizedDayMarker, result.dayRemark) {
        mutableStateOf(result.dayRemark)
    }
    var events by remember(result.normalizedDayMarker, result.events) {
        mutableStateOf(result.events)
    }
    var editingDayRemark by remember { mutableStateOf(false) }
    var editingTimeIndex by remember { mutableStateOf<Int?>(null) }
    var choosingActivityIndex by remember { mutableStateOf<Int?>(null) }
    var editingActivityRemarkIndex by remember { mutableStateOf<Int?>(null) }
    var deletingActivityIndex by remember { mutableStateOf<Int?>(null) }

    Column(verticalArrangement = Arrangement.spacedBy(10.dp)) {
        OutlinedCard(modifier = Modifier.fillMaxWidth()) {
            Row(
                modifier = Modifier.padding(12.dp).fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text(
                    text = dayRemark.ifBlank {
                        stringResource(R.string.txt_day_edit_day_remark)
                    },
                    modifier = Modifier.weight(1f),
                    style = MaterialTheme.typography.bodySmall,
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )
                TextButton(onClick = { editingDayRemark = true }) {
                    Text(stringResource(R.string.txt_day_edit_remark))
                }
            }
        }
        if (events.isEmpty()) {
            Text(
                text = stringResource(R.string.txt_day_edit_empty),
                style = MaterialTheme.typography.bodyMedium,
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
        }
        events.forEachIndexed { index, event ->
            OutlinedCard(modifier = Modifier.fillMaxWidth()) {
                Column(
                    modifier = Modifier.padding(12.dp),
                    verticalArrangement = Arrangement.spacedBy(8.dp)
                ) {
                    Row(
                        modifier = Modifier.fillMaxWidth(),
                        horizontalArrangement = Arrangement.SpaceBetween,
                        verticalAlignment = Alignment.CenterVertically
                    ) {
                        Text(
                            text = formatTxtDayEventTime(event, use12HourTime),
                            style = MaterialTheme.typography.titleMedium
                        )
                        TextButton(onClick = { editingTimeIndex = index }) {
                            Text(stringResource(R.string.txt_day_edit_time))
                        }
                    }
                    Row(
                        modifier = Modifier.fillMaxWidth(),
                        horizontalArrangement = Arrangement.SpaceBetween,
                        verticalAlignment = Alignment.CenterVertically
                    ) {
                        Text(
                            text = event.activityToken,
                            modifier = Modifier.weight(1f),
                            style = MaterialTheme.typography.bodyLarge
                        )
                        TextButton(onClick = { choosingActivityIndex = index }) {
                            Text(stringResource(R.string.txt_day_edit_activity))
                        }
                    }
                    Row(
                        modifier = Modifier.fillMaxWidth(),
                        horizontalArrangement = Arrangement.SpaceBetween,
                        verticalAlignment = Alignment.CenterVertically
                    ) {
                        Text(
                            text = event.remark,
                            modifier = Modifier.weight(1f),
                            style = MaterialTheme.typography.bodySmall,
                            color = MaterialTheme.colorScheme.onSurfaceVariant
                        )
                        TextButton(onClick = { editingActivityRemarkIndex = index }) {
                            Text(stringResource(R.string.txt_day_edit_remark))
                        }
                    }
                    Row(
                        modifier = Modifier.fillMaxWidth(),
                        horizontalArrangement = Arrangement.End
                    ) {
                        IconButton(onClick = { deletingActivityIndex = index }) {
                            Icon(
                                imageVector = Icons.Outlined.Delete,
                                contentDescription = stringResource(
                                    R.string.txt_day_edit_delete_activity
                                ),
                                tint = MaterialTheme.colorScheme.error
                            )
                        }
                    }
                }
            }
        }
    }

    editingTimeIndex?.let { index ->
        val event = events.getOrNull(index) ?: return@let
        val nextTimelineEvent = events.drop(index + 1).firstOrNull {
            it.startTimelineSeconds != null
        }
        TxtDayTimeEditSheet(
            event = event,
            previousEndTimelineSeconds = event.previousEndTimelineSeconds,
            nextStartTimelineSeconds = event.nextStartTimelineSeconds,
            nextEventIsInterval = nextTimelineEvent?.isInterval == true,
            onDismiss = { editingTimeIndex = null },
            onApply = { edited ->
                val updatedEvents = events.toMutableList().also { it[index] = edited }
                events = updatedEvents
                onApply(dayRemark, updatedEvents)
                editingTimeIndex = null
            }
        )
    }
    choosingActivityIndex?.let { index ->
        CanonicalActivityPickerScreen(
            isLoading = catalogLoading,
            roots = roots,
            statusText = catalogStatusText,
            displayMode = RecordFrequentOutputMode.CANONICAL,
            target = CanonicalBrowserTarget.TXT_DAY_EDIT,
            collapsedRootPaths = collapsedRootPaths,
            orderedRootPaths = orderedRootPaths,
            onDismissRequest = { choosingActivityIndex = null },
            onDisplayModeChange = {},
            onCollapsedRootPathsChange = onCollapsedRootPathsChange,
            onOrderedRootPathsChange = onOrderedRootPathsChange,
            onCanonicalEntryClick = { entry ->
                val updatedEvents = events.toMutableList().also { current ->
                    current[index] = current[index].copy(activityToken = entry.canonicalPath)
                }
                events = updatedEvents
                onApply(dayRemark, updatedEvents)
                choosingActivityIndex = null
            }
        )
    }
    if (editingDayRemark) {
        TxtDayRemarkEditSheet(
            title = stringResource(R.string.txt_day_edit_day_remark_title),
            initialRemark = dayRemark,
            onDismiss = { editingDayRemark = false },
            onApply = { editedRemark ->
                dayRemark = editedRemark
                onApply(dayRemark, events)
                editingDayRemark = false
            }
        )
    }
    editingActivityRemarkIndex?.let { index ->
        val event = events.getOrNull(index) ?: return@let
        TxtDayRemarkEditSheet(
            title = stringResource(R.string.txt_day_edit_activity_remark_title),
            initialRemark = event.remark,
            onDismiss = { editingActivityRemarkIndex = null },
            onApply = { editedRemark ->
                val updatedEvents = events.toMutableList().also { current ->
                    current[index] = current[index].copy(remark = editedRemark)
                }
                events = updatedEvents
                onApply(dayRemark, updatedEvents)
                editingActivityRemarkIndex = null
            }
        )
    }
    deletingActivityIndex?.let { index ->
        val event = events.getOrNull(index) ?: return@let
        AlertDialog(
            onDismissRequest = { deletingActivityIndex = null },
            title = { Text(stringResource(R.string.txt_day_edit_delete_activity_title)) },
            text = {
                Text(
                    stringResource(
                        R.string.txt_day_edit_delete_activity_message,
                        event.activityToken
                    )
                )
            },
            confirmButton = {
                TextButton(
                    onClick = {
                        val updatedEvents = events.toMutableList().also { it.removeAt(index) }
                        events = updatedEvents
                        onApply(dayRemark, updatedEvents)
                        deletingActivityIndex = null
                    }
                ) {
                    Text(
                        text = stringResource(R.string.txt_day_edit_delete_activity),
                        color = MaterialTheme.colorScheme.error
                    )
                }
            },
            dismissButton = {
                TextButton(onClick = { deletingActivityIndex = null }) {
                    Text(stringResource(R.string.txt_action_close))
                }
            }
        )
    }
}

@Composable
@OptIn(ExperimentalMaterial3Api::class)
internal fun TxtActivityFindReplace(
    visible: Boolean,
    events: List<TxtDayEditEvent>,
    dayRemark: String,
    roots: List<CanonicalPathNode>,
    catalogLoading: Boolean,
    catalogStatusText: String,
    collapsedRootPaths: Set<String>,
    orderedRootPaths: List<String>,
    onCollapsedRootPathsChange: (Set<String>) -> Unit,
    onOrderedRootPathsChange: (List<String>) -> Unit,
    onCanonicalCatalogRequested: () -> Unit = {},
    onDismiss: () -> Unit,
    onReplaceDayActivity: suspend (String, String, String, List<TxtDayEditEvent>) ->
        TxtDayActivityReplacementResult,
    onLoadMonthActivities: suspend () -> TxtMonthActivityEditsResult,
    onReplaceMonthActivity: suspend (TxtMonthActivityEditSnapshot, String, String) ->
        TxtMonthActivityReplacementResult
) {
    if (!visible) return

    var scope by remember { mutableStateOf(TxtActivityFindReplaceScope.DAY) }
    val onScopeChange = { value: TxtActivityFindReplaceScope -> scope = value }
    if (scope == TxtActivityFindReplaceScope.DAY) {
        TxtDayActivityFindReplace(
            visible = true, events = events, dayRemark = dayRemark, roots = roots,
            catalogLoading = catalogLoading, catalogStatusText = catalogStatusText,
            collapsedRootPaths = collapsedRootPaths, orderedRootPaths = orderedRootPaths,
            onCollapsedRootPathsChange = onCollapsedRootPathsChange,
            onOrderedRootPathsChange = onOrderedRootPathsChange,
            onCanonicalCatalogRequested = onCanonicalCatalogRequested,
            onScopeChange = onScopeChange, onDismiss = onDismiss,
            onReplaceActivity = onReplaceDayActivity
        )
    } else {
        TxtMonthActivityFindReplace(
            visible = true, roots = roots, catalogLoading = catalogLoading,
            catalogStatusText = catalogStatusText, collapsedRootPaths = collapsedRootPaths,
            orderedRootPaths = orderedRootPaths,
            onCollapsedRootPathsChange = onCollapsedRootPathsChange,
            onOrderedRootPathsChange = onOrderedRootPathsChange,
            onCanonicalCatalogRequested = onCanonicalCatalogRequested,
            onScopeChange = onScopeChange, onLoad = onLoadMonthActivities,
            onDismiss = onDismiss, onReplaceActivity = onReplaceMonthActivity
        )
    }
}

internal enum class TxtActivityFindReplaceScope { DAY, MONTH }

@Composable
@OptIn(ExperimentalMaterial3Api::class)
internal fun TxtDayActivityFindReplace(
    visible: Boolean,
    events: List<TxtDayEditEvent>,
    dayRemark: String,
    roots: List<CanonicalPathNode>,
    catalogLoading: Boolean,
    catalogStatusText: String,
    collapsedRootPaths: Set<String>,
    orderedRootPaths: List<String>,
    onCollapsedRootPathsChange: (Set<String>) -> Unit,
    onOrderedRootPathsChange: (List<String>) -> Unit,
    onCanonicalCatalogRequested: () -> Unit = {},
    onScopeChange: (TxtActivityFindReplaceScope) -> Unit,
    onDismiss: () -> Unit,
    onReplaceActivity: suspend (
        sourceActivityToken: String,
        targetActivityToken: String,
        dayRemark: String,
        events: List<TxtDayEditEvent>
    ) -> TxtDayActivityReplacementResult
) {
    if (!visible) {
        return
    }
    var stage by remember { mutableStateOf(TxtDayActivityReplaceStage.FIND_SOURCE) }
    var sourceActivityToken by remember { mutableStateOf<String?>(null) }
    var targetActivityToken by remember { mutableStateOf<String?>(null) }
    var replacementInProgress by remember { mutableStateOf(false) }
    var replacementError by remember { mutableStateOf("") }
    val coroutineScope = androidx.compose.runtime.rememberCoroutineScope()
    val activityOccurrences = remember(events, roots) {
        buildTxtDayActivitySearchOccurrences(events, roots)
    }
    LaunchedEffect(stage, roots) {
        if (stage == TxtDayActivityReplaceStage.SELECT_TARGET && roots.isEmpty()) {
            onCanonicalCatalogRequested()
        }
    }
    when (stage) {
        TxtDayActivityReplaceStage.FIND_SOURCE -> TxtDayActivitySourcePage(
            occurrences = activityOccurrences,
            scope = TxtActivityFindReplaceScope.DAY,
            onScopeChange = onScopeChange,
            onDismiss = onDismiss,
            onSelect = { selectedActivityToken ->
                sourceActivityToken = selectedActivityToken
                stage = TxtDayActivityReplaceStage.SELECT_TARGET
            }
        )

        TxtDayActivityReplaceStage.SELECT_TARGET -> CanonicalActivityPickerScreen(
            isLoading = catalogLoading && roots.isEmpty(),
            roots = roots,
            statusText = catalogStatusText,
            displayMode = RecordFrequentOutputMode.CANONICAL,
            target = CanonicalBrowserTarget.TXT_DAY_EDIT,
            collapsedRootPaths = collapsedRootPaths,
            orderedRootPaths = orderedRootPaths,
            onDismissRequest = onDismiss,
            onDisplayModeChange = {},
            onCollapsedRootPathsChange = onCollapsedRootPathsChange,
            onOrderedRootPathsChange = onOrderedRootPathsChange,
            onCanonicalEntryClick = { entry ->
                targetActivityToken = entry.canonicalPath
                stage = TxtDayActivityReplaceStage.CONFIRM
            }
        )

        TxtDayActivityReplaceStage.CONFIRM -> {
            val occurrenceCount = activityOccurrences
                .firstOrNull { it.activityToken == sourceActivityToken }
                ?.occurrenceCount ?: 0
            AlertDialog(
                onDismissRequest = {
                    if (!replacementInProgress) {
                        onDismiss()
                    }
                },
                title = { Text(stringResource(R.string.txt_day_edit_find_replace_confirm_title)) },
                text = {
                    Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
                        Text(
                            pluralStringResource(
                                R.plurals.txt_day_edit_find_replace_confirm_message,
                                occurrenceCount,
                                occurrenceCount,
                                sourceActivityToken.orEmpty(),
                                targetActivityToken.orEmpty()
                            )
                        )
                        if (replacementError.isNotBlank()) {
                            Text(
                                text = replacementError,
                                style = MaterialTheme.typography.bodySmall,
                                color = MaterialTheme.colorScheme.error
                            )
                        }
                    }
                },
                confirmButton = {
                    TextButton(
                        enabled = !replacementInProgress &&
                            sourceActivityToken != null &&
                            targetActivityToken != null,
                        onClick = {
                            coroutineScope.launch {
                                replacementInProgress = true
                                val replacement = onReplaceActivity(
                                    requireNotNull(sourceActivityToken),
                                    requireNotNull(targetActivityToken),
                                    dayRemark,
                                    events
                                )
                                replacementInProgress = false
                                if (replacement.ok) {
                                    onDismiss()
                                } else {
                                    replacementError = replacement.message
                                }
                            }
                        }
                    ) {
                        Text(stringResource(R.string.txt_day_edit_find_replace_action))
                    }
                },
                dismissButton = {
                    TextButton(
                        enabled = !replacementInProgress,
                        onClick = onDismiss
                    ) {
                        Text(stringResource(R.string.txt_action_close))
                    }
                }
            )
        }
    }
}

private enum class TxtDayActivityReplaceStage {
    FIND_SOURCE,
    SELECT_TARGET,
    CONFIRM
}

@Composable
private fun TxtDayActivitySourcePage(
    occurrences: List<TxtDayActivitySearchOccurrence>,
    scope: TxtActivityFindReplaceScope,
    onScopeChange: (TxtActivityFindReplaceScope) -> Unit,
    searchLabelRes: Int = R.string.txt_day_edit_find_replace_search_label,
    sourceHintRes: Int = R.string.txt_day_edit_find_replace_source_hint,
    noMatchesRes: Int = R.string.txt_day_edit_find_replace_no_matches,
    statusMessage: String = "",
    isStatusError: Boolean = false,
    onDismiss: () -> Unit,
    onSelect: (String) -> Unit
) {
    var query by remember { mutableStateOf("") }
    val matchedOccurrences = remember(query, occurrences) {
        occurrences.filter { occurrence ->
            occurrence.searchTokens.any { token ->
                token.contains(query, ignoreCase = true)
            }
        }
    }
    FullscreenPage(onDismissRequest = onDismiss) {
            Column(
                modifier = Modifier
                    .padding(24.dp)
                    .verticalScroll(rememberScrollState()),
                verticalArrangement = Arrangement.spacedBy(12.dp)
            ) {
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.SpaceBetween,
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Text(
                        text = stringResource(R.string.txt_day_edit_find_replace_source_title),
                        style = MaterialTheme.typography.headlineSmall
                    )
                    IconButton(onClick = onDismiss) {
                        Icon(
                            imageVector = Icons.Default.Close,
                            contentDescription = stringResource(R.string.txt_action_close)
                        )
                    }
                }
                PrimaryTabRow(selectedTabIndex = scope.ordinal) {
                    Tab(
                        selected = scope == TxtActivityFindReplaceScope.DAY,
                        onClick = { onScopeChange(TxtActivityFindReplaceScope.DAY) },
                        text = { Text(stringResource(R.string.txt_raw_editor_tab_day)) }
                    )
                    Tab(
                        selected = scope == TxtActivityFindReplaceScope.MONTH,
                        onClick = { onScopeChange(TxtActivityFindReplaceScope.MONTH) },
                        text = { Text(stringResource(R.string.txt_raw_editor_tab_month)) }
                    )
                }
                OutlinedTextField(
                    value = query,
                    onValueChange = { query = it },
                    modifier = Modifier.fillMaxWidth(),
                    label = { Text(stringResource(searchLabelRes)) },
                    singleLine = true
                )
                if (query.isBlank()) {
                    Text(
                        text = if (statusMessage.isBlank()) {
                            stringResource(sourceHintRes)
                        } else {
                            statusMessage
                        },
                        style = MaterialTheme.typography.bodyMedium,
                        color = if (isStatusError) {
                            MaterialTheme.colorScheme.error
                        } else {
                            MaterialTheme.colorScheme.onSurfaceVariant
                        }
                    )
                } else if (matchedOccurrences.isEmpty()) {
                    Text(
                        text = stringResource(noMatchesRes),
                        style = MaterialTheme.typography.bodyMedium,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                }
                matchedOccurrences.forEach { occurrence ->
                    OutlinedCard(
                        modifier = Modifier.fillMaxWidth(),
                        onClick = { onSelect(occurrence.activityToken) }
                    ) {
                        Row(
                            modifier = Modifier.padding(12.dp).fillMaxWidth(),
                            horizontalArrangement = Arrangement.SpaceBetween,
                            verticalAlignment = Alignment.CenterVertically
                        ) {
                            Text(
                                text = occurrence.activityToken,
                                modifier = Modifier.weight(1f),
                                style = MaterialTheme.typography.bodyLarge
                            )
                            Text(
                                text = pluralStringResource(
                                    R.plurals.txt_day_edit_find_replace_occurrences,
                                    occurrence.occurrenceCount,
                                    occurrence.occurrenceCount
                                ),
                                style = MaterialTheme.typography.bodySmall,
                                color = MaterialTheme.colorScheme.onSurfaceVariant
                            )
                        }
                    }
                }
            }
    }
}

@Composable
@OptIn(ExperimentalMaterial3Api::class)
internal fun TxtMonthActivityFindReplace(
    visible: Boolean,
    roots: List<CanonicalPathNode>,
    catalogLoading: Boolean,
    catalogStatusText: String,
    collapsedRootPaths: Set<String>,
    orderedRootPaths: List<String>,
    onCollapsedRootPathsChange: (Set<String>) -> Unit,
    onOrderedRootPathsChange: (List<String>) -> Unit,
    onCanonicalCatalogRequested: () -> Unit = {},
    onScopeChange: (TxtActivityFindReplaceScope) -> Unit,
    onLoad: suspend () -> TxtMonthActivityEditsResult,
    onDismiss: () -> Unit,
    onReplaceActivity: suspend (
        TxtMonthActivityEditSnapshot,
        String,
        String
    ) -> TxtMonthActivityReplacementResult
) {
    if (!visible) {
        return
    }
    var snapshot by remember { mutableStateOf<TxtMonthActivityEditSnapshot?>(null) }
    var loadError by remember { mutableStateOf("") }
    var stage by remember { mutableStateOf(TxtDayActivityReplaceStage.FIND_SOURCE) }
    var sourceActivityToken by remember { mutableStateOf<String?>(null) }
    var targetActivityToken by remember { mutableStateOf<String?>(null) }
    var replacementInProgress by remember { mutableStateOf(false) }
    var replacementError by remember { mutableStateOf("") }
    val coroutineScope = androidx.compose.runtime.rememberCoroutineScope()

    LaunchedEffect(Unit) {
        val loaded = onLoad()
        snapshot = loaded.snapshot
        loadError = if (loaded.ok) "" else loaded.message
    }

    if (snapshot == null) {
        TxtDayActivitySourcePage(
            occurrences = emptyList(),
            scope = TxtActivityFindReplaceScope.MONTH,
            onScopeChange = onScopeChange,
            searchLabelRes = R.string.txt_month_edit_find_replace_search_label,
            sourceHintRes = R.string.txt_month_edit_find_replace_source_hint,
            noMatchesRes = R.string.txt_month_edit_find_replace_no_matches,
            statusMessage = if (loadError.isBlank()) {
                stringResource(R.string.record_hint_loading)
            } else {
                loadError
            },
            isStatusError = loadError.isNotBlank(),
            onDismiss = onDismiss,
            onSelect = {}
        )
        return
    }

    val activityOccurrences = remember(snapshot, roots) {
        buildTxtDayActivitySearchOccurrences(
            events = requireNotNull(snapshot).dayEdits.flatMap(TxtMonthDayEdit::events),
            roots = roots
        )
    }
    LaunchedEffect(stage, roots) {
        if (stage == TxtDayActivityReplaceStage.SELECT_TARGET && roots.isEmpty()) {
            onCanonicalCatalogRequested()
        }
    }
    when (stage) {
        TxtDayActivityReplaceStage.FIND_SOURCE -> TxtDayActivitySourcePage(
            occurrences = activityOccurrences,
            scope = TxtActivityFindReplaceScope.MONTH,
            onScopeChange = onScopeChange,
            searchLabelRes = R.string.txt_month_edit_find_replace_search_label,
            sourceHintRes = R.string.txt_month_edit_find_replace_source_hint,
            noMatchesRes = R.string.txt_month_edit_find_replace_no_matches,
            onDismiss = onDismiss,
            onSelect = { selectedActivityToken ->
                sourceActivityToken = selectedActivityToken
                stage = TxtDayActivityReplaceStage.SELECT_TARGET
            }
        )

        TxtDayActivityReplaceStage.SELECT_TARGET -> CanonicalActivityPickerScreen(
            isLoading = catalogLoading && roots.isEmpty(),
            roots = roots,
            statusText = catalogStatusText,
            displayMode = RecordFrequentOutputMode.CANONICAL,
            target = CanonicalBrowserTarget.TXT_DAY_EDIT,
            collapsedRootPaths = collapsedRootPaths,
            orderedRootPaths = orderedRootPaths,
            onDismissRequest = onDismiss,
            onDisplayModeChange = {},
            onCollapsedRootPathsChange = onCollapsedRootPathsChange,
            onOrderedRootPathsChange = onOrderedRootPathsChange,
            onCanonicalEntryClick = { entry ->
                targetActivityToken = entry.canonicalPath
                stage = TxtDayActivityReplaceStage.CONFIRM
            }
        )

        TxtDayActivityReplaceStage.CONFIRM -> {
            val occurrenceCount = activityOccurrences
                .firstOrNull { it.activityToken == sourceActivityToken }
                ?.occurrenceCount ?: 0
            AlertDialog(
                onDismissRequest = {
                    if (!replacementInProgress) {
                        onDismiss()
                    }
                },
                title = { Text(stringResource(R.string.txt_day_edit_find_replace_confirm_title)) },
                text = {
                    Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
                        Text(
                            pluralStringResource(
                                R.plurals.txt_month_edit_find_replace_confirm_message,
                                occurrenceCount,
                                occurrenceCount,
                                sourceActivityToken.orEmpty(),
                                targetActivityToken.orEmpty()
                            )
                        )
                        if (replacementError.isNotBlank()) {
                            Text(
                                text = replacementError,
                                style = MaterialTheme.typography.bodySmall,
                                color = MaterialTheme.colorScheme.error
                            )
                        }
                    }
                },
                confirmButton = {
                    TextButton(
                        enabled = !replacementInProgress &&
                            sourceActivityToken != null &&
                            targetActivityToken != null,
                        onClick = {
                            coroutineScope.launch {
                                replacementInProgress = true
                                val replacement = onReplaceActivity(
                                    requireNotNull(snapshot),
                                    requireNotNull(sourceActivityToken),
                                    requireNotNull(targetActivityToken)
                                )
                                replacementInProgress = false
                                if (replacement.ok) {
                                    onDismiss()
                                } else {
                                    replacementError = replacement.message
                                }
                            }
                        }
                    ) {
                        Text(stringResource(R.string.txt_day_edit_find_replace_action))
                    }
                },
                dismissButton = {
                    TextButton(
                        enabled = !replacementInProgress,
                        onClick = onDismiss
                    ) {
                        Text(stringResource(R.string.txt_action_close))
                    }
                }
            )
        }
    }
}

internal data class TxtDayActivitySearchOccurrence(
    val activityToken: String,
    val occurrenceCount: Int,
    val searchTokens: Set<String>
)

internal fun buildTxtDayActivitySearchOccurrences(
    events: List<TxtDayEditEvent>,
    roots: List<CanonicalPathNode>
): List<TxtDayActivitySearchOccurrence> {
    val occurrenceCounts = linkedMapOf<String, Int>()
    events.forEach { event ->
        occurrenceCounts[event.activityToken] = (occurrenceCounts[event.activityToken] ?: 0) + 1
    }
    val catalogEntries = roots.flatMap(::flattenCanonicalCatalogEntries)
    return occurrenceCounts.map { (activityToken, occurrenceCount) ->
        val searchTokens = linkedSetOf(activityToken)
        catalogEntries
            .filter { entry ->
                entry.canonicalPath == activityToken || entry.aliases.contains(activityToken)
            }
            .forEach { entry ->
                searchTokens += entry.canonicalPath
                searchTokens += entry.aliases.filter(String::isNotBlank)
            }
        TxtDayActivitySearchOccurrence(
            activityToken = activityToken,
            occurrenceCount = occurrenceCount,
            searchTokens = searchTokens
        )
    }
}

private fun flattenCanonicalCatalogEntries(root: CanonicalPathNode): List<CanonicalCatalogEntry> =
    root.entries + root.children.flatMap(::flattenCanonicalCatalogEntries)

@Composable
@OptIn(ExperimentalMaterial3Api::class)
private fun TxtDayRemarkEditSheet(
    title: String,
    initialRemark: String,
    onDismiss: () -> Unit,
    onApply: (String) -> Unit
) {
    var remark by remember(initialRemark) { mutableStateOf(initialRemark) }
    val sheetState = rememberModalBottomSheetState(skipPartiallyExpanded = true)
    ModalBottomSheet(
        onDismissRequest = onDismiss,
        sheetState = sheetState
    ) {
        Column(
            modifier = Modifier.padding(start = 24.dp, top = 8.dp, end = 24.dp, bottom = 24.dp),
            verticalArrangement = Arrangement.spacedBy(16.dp)
        ) {
            Text(text = title, style = MaterialTheme.typography.headlineSmall)
            OutlinedTextField(
                value = remark,
                onValueChange = { remark = it },
                modifier = Modifier.fillMaxWidth(),
                label = { Text(stringResource(R.string.txt_day_edit_remark)) },
                minLines = 3
            )
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.End
            ) {
                TextButton(onClick = onDismiss) {
                    Text(stringResource(R.string.txt_action_close))
                }
                Button(onClick = { onApply(remark) }) {
                    Text(stringResource(R.string.txt_day_edit_save))
                }
            }
        }
    }
}

@Composable
@OptIn(ExperimentalMaterial3Api::class)
private fun TxtDayTimeEditSheet(
    event: TxtDayEditEvent,
    previousEndTimelineSeconds: Int?,
    nextStartTimelineSeconds: Int?,
    nextEventIsInterval: Boolean,
    onDismiss: () -> Unit,
    onApply: (TxtDayEditEvent) -> Unit
) {
    var startTimeline by remember(event.startTimelineSeconds) {
        mutableStateOf(event.startTimelineSeconds ?: parseClockSeconds(event.startTime))
    }
    var endTimeline by remember(event.endTimelineSeconds) {
        mutableStateOf(event.endTimelineSeconds ?: parseClockSeconds(event.endTime))
    }
    val lowerBoundary = previousEndTimelineSeconds
    val upperBoundary = nextStartTimelineSeconds
    val sheetState = rememberModalBottomSheetState(skipPartiallyExpanded = true)
    ModalBottomSheet(
        onDismissRequest = onDismiss,
        sheetState = sheetState
    ) {
        Column(
            modifier = Modifier.padding(start = 24.dp, top = 8.dp, end = 24.dp, bottom = 24.dp),
            verticalArrangement = Arrangement.spacedBy(16.dp)
        ) {
            Text(
                text = stringResource(R.string.txt_day_edit_time_title),
                style = MaterialTheme.typography.headlineSmall
            )
            if (event.isInterval) {
                Text(
                    text = stringResource(R.string.record_label_interval_start),
                    style = MaterialTheme.typography.labelLarge
                )
                TimelineTimeOfDayEditor(
                    valueTimelineSeconds = startTimeline,
                    minimumTimelineSeconds = lowerBoundary ?: 0,
                    maximumTimelineSeconds = (endTimeline - 1).coerceAtLeast(
                        lowerBoundary ?: 0
                    ),
                    onValueChange = { startTimeline = it }
                )
            }
            Text(
                text = if (event.isInterval) {
                    stringResource(R.string.record_label_interval_end)
                } else {
                    stringResource(R.string.txt_day_edit_time)
                },
                style = MaterialTheme.typography.labelLarge
            )
            TimelineTimeOfDayEditor(
                valueTimelineSeconds = endTimeline,
                minimumTimelineSeconds = if (event.isInterval) {
                    startTimeline + 1
                } else {
                    (lowerBoundary ?: -1) + 1
                },
                maximumTimelineSeconds = upperBoundary?.let { boundary ->
                    if (event.isInterval || nextEventIsInterval) boundary else boundary - 1
                } ?: if (event.isInterval) {
                    startTimeline + SECONDS_PER_DAY - 1
                } else {
                    ((endTimeline / SECONDS_PER_DAY) + 1) * SECONDS_PER_DAY - 1
                },
                onValueChange = { endTimeline = it }
            )
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.End
            ) {
                TextButton(onClick = onDismiss) {
                    Text(stringResource(R.string.txt_action_close))
                }
                Button(
                    onClick = {
                        onApply(
                            event.copy(
                                startTime = formatClockSeconds(startTimeline),
                                endTime = formatClockSeconds(endTimeline),
                                startTimelineSeconds = startTimeline,
                                endTimelineSeconds = endTimeline
                            )
                        )
                    }
                ) {
                    Text(stringResource(R.string.txt_day_edit_save))
                }
            }
        }
    }
}

@Composable
private fun TimelineTimeOfDayEditor(
    valueTimelineSeconds: Int,
    minimumTimelineSeconds: Int,
    maximumTimelineSeconds: Int,
    onValueChange: (Int) -> Unit
) {
    val safeMinimum = minimumTimelineSeconds.coerceAtLeast(0)
    val safeMaximum = maximumTimelineSeconds.coerceAtLeast(safeMinimum)
    val selected = valueTimelineSeconds.coerceIn(safeMinimum, safeMaximum)
    val hour = selected / SECONDS_PER_HOUR
    val minute = (selected % SECONDS_PER_HOUR) / SECONDS_PER_MINUTE
    val second = selected % SECONDS_PER_MINUTE
    val minimumHour = safeMinimum / SECONDS_PER_HOUR
    val maximumHour = safeMaximum / SECONDS_PER_HOUR
    fun minimumMinuteForHour(candidateHour: Int): Int =
        if (candidateHour == minimumHour) {
            (safeMinimum % SECONDS_PER_HOUR) / SECONDS_PER_MINUTE
        } else {
            0
        }
    fun maximumMinuteForHour(candidateHour: Int): Int =
        if (candidateHour == maximumHour) {
            (safeMaximum % SECONDS_PER_HOUR) / SECONDS_PER_MINUTE
        } else {
            59
        }
    val minimumMinute = minimumMinuteForHour(hour)
    val maximumMinute = maximumMinuteForHour(hour)
    fun minimumSecond(candidateMinute: Int): Int =
        if (hour == minimumHour && candidateMinute == minimumMinute) {
            safeMinimum % SECONDS_PER_MINUTE
        } else {
            0
        }
    fun maximumSecond(candidateMinute: Int): Int =
        if (hour == maximumHour && candidateMinute == maximumMinute) {
            safeMaximum % SECONDS_PER_MINUTE
        } else {
            59
        }
    val minimumSecond = minimumSecond(minute)
    val maximumSecond = maximumSecond(minute)
    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.spacedBy(8.dp)
    ) {
        WheelNumberPicker(
            label = stringResource(R.string.record_time_hour),
            value = hour,
            values = minimumHour..maximumHour,
            valueText = { "%02d".format(it % 24) },
            modifier = Modifier.weight(1f),
            onValueChange = { nextHour ->
                val nextMinute = minute.coerceIn(
                    minimumMinuteForHour(nextHour), maximumMinuteForHour(nextHour)
                )
                val nextSecond = second.coerceIn(
                    if (nextHour == minimumHour &&
                        nextMinute == minimumMinuteForHour(nextHour)
                    ) {
                        safeMinimum % SECONDS_PER_MINUTE
                    } else 0,
                    if (nextHour == maximumHour &&
                        nextMinute == maximumMinuteForHour(nextHour)
                    ) {
                        safeMaximum % SECONDS_PER_MINUTE
                    } else 59
                )
                onValueChange(nextHour * SECONDS_PER_HOUR +
                    nextMinute * SECONDS_PER_MINUTE + nextSecond)
            }
        )
        WheelNumberPicker(
            label = stringResource(R.string.record_time_minute),
            value = minute,
            values = minimumMinute..maximumMinute,
            modifier = Modifier.weight(1f),
            onValueChange = { nextMinute ->
                val nextSecond = second.coerceIn(
                    minimumSecond(nextMinute), maximumSecond(nextMinute)
                )
                onValueChange(hour * SECONDS_PER_HOUR +
                    nextMinute * SECONDS_PER_MINUTE + nextSecond)
            }
        )
        WheelNumberPicker(
            label = stringResource(R.string.record_time_second),
            value = second,
            values = minimumSecond..maximumSecond,
            modifier = Modifier.weight(1f),
            onValueChange = { nextSecond ->
                onValueChange(hour * SECONDS_PER_HOUR +
                    minute * SECONDS_PER_MINUTE + nextSecond)
            }
        )
    }
}

private fun parseClockSeconds(value: String): Int {
    val digits = value.filter(Char::isDigit).padStart(6, '0').take(6)
    return digits.substring(0, 2).toInt() * SECONDS_PER_HOUR +
        digits.substring(2, 4).toInt() * SECONDS_PER_MINUTE +
        digits.substring(4, 6).toInt()
}

internal fun formatClockSeconds(value: Int): String {
    val clockValue = ((value % SECONDS_PER_DAY) + SECONDS_PER_DAY) % SECONDS_PER_DAY
    val hour = clockValue / SECONDS_PER_HOUR
    val minute = (clockValue % SECONDS_PER_HOUR) / SECONDS_PER_MINUTE
    val second = clockValue % SECONDS_PER_MINUTE
    // Structured day-edit events use ISO clock strings on the Android/Core
    // boundary. Core validates them, then serializes Raw TXT as HHMMSS.
    return "%02d:%02d:%02d".format(hour, minute, second)
}

private const val SECONDS_PER_MINUTE = 60
private const val SECONDS_PER_HOUR = 60 * SECONDS_PER_MINUTE
private const val SECONDS_PER_DAY = 24 * SECONDS_PER_HOUR

internal fun formatTxtDayEventTime(
    event: TxtDayEditEvent,
    use12HourTime: Boolean = false
): String {
    fun format(value: String): String = formatDisplayClockTime(
        formatIsoClockTime(value),
        use12HourTime
    )
    return if (event.isInterval) {
        "${format(event.startTime)} – ${format(event.endTime)}"
    } else {
        format(event.endTime)
    }
}
