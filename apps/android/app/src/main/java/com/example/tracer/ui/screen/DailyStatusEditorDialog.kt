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
import androidx.compose.material.icons.filled.Add
import androidx.compose.material.icons.filled.Close
import androidx.compose.material.icons.filled.Delete
import androidx.compose.material.icons.filled.Edit
import androidx.compose.material3.Card
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import androidx.compose.ui.window.Dialog
import androidx.compose.ui.window.DialogProperties
import com.example.tracer.data.DailyStatusConfig
import com.example.tracer.data.DailyStatusConfigStore
import com.example.tracer.data.DailyStatusDefinition
import com.example.tracer.data.UserPreferencesRepository
import kotlinx.coroutines.flow.first
import kotlinx.coroutines.launch

@Composable
internal fun DailyStatusEditorDialog(
    userPreferencesRepository: UserPreferencesRepository,
    runtimeInitializer: RuntimeInitializer,
    insightsMode: InsightsMode,
    statusValues: List<InsightsStatusValue>,
    recordUiState: RecordUiState,
    recordViewModel: RecordViewModel,
    onConfigSaved: () -> Unit,
    onDismissRequest: () -> Unit
) {
    val scopeLabel = stringResource(insightsMode.statusEditorScopeResId())
    var config by remember { mutableStateOf(DailyStatusConfig()) }
    var isLoading by remember { mutableStateOf(true) }
    var isSaving by remember { mutableStateOf(false) }
    var errorMessage by remember { mutableStateOf("") }
    var editingId by remember { mutableStateOf<String?>(null) }
    var parentPickerStatusId by remember { mutableStateOf<String?>(null) }
    var selectedParentOverride by remember { mutableStateOf<String?>(null) }
    val coroutineScope = rememberCoroutineScope()

    LaunchedEffect(userPreferencesRepository, insightsMode) {
        config = userPreferencesRepository.insightsStatusConfigs.first()[insightsMode]
        isLoading = false
    }

    fun persistConfig(next: DailyStatusConfig) {
        config = next
        isSaving = true
        errorMessage = ""
        coroutineScope.launch {
            runCatching {
                userPreferencesRepository.setStatusConfig(insightsMode, next)
                val reload = runtimeInitializer.initializeRuntime()
                check(reload.initialized) {
                    "Daily statuses were saved but runtime reload failed."
                }
            }.onSuccess {
                isSaving = false
                onConfigSaved()
            }.onFailure { error ->
                errorMessage = error.message ?: "Cannot save daily statuses."
                isSaving = false
            }
        }
    }

    fun updateStatus(next: DailyStatusDefinition) {
        val oldId = editingId
        if (config.statuses.any { it.id == next.id && it.id != oldId }) {
            errorMessage = "A Daily status already uses this parent."
            return
        }
        persistConfig(config.copy(
            statuses = config.statuses.map { if (it.id == oldId) next else it }
        ))
        editingId = null
    }

    Dialog(
        onDismissRequest = onDismissRequest,
        properties = DialogProperties(usePlatformDefaultWidth = false)
    ) {
        Surface(modifier = Modifier.fillMaxSize(), color = MaterialTheme.colorScheme.surface) {
            Column(
                modifier = Modifier
                    .fillMaxSize()
                    .padding(horizontal = 24.dp, vertical = 16.dp),
                verticalArrangement = Arrangement.spacedBy(12.dp)
            ) {
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    verticalAlignment = Alignment.CenterVertically,
                    horizontalArrangement = Arrangement.SpaceBetween
                ) {
                    Text(
                        text = stringResource(R.string.insights_status_editor_title, scopeLabel),
                        style = MaterialTheme.typography.titleLarge
                    )
                    IconButton(onClick = onDismissRequest) {
                        Icon(Icons.Default.Close, contentDescription = stringResource(R.string.daily_status_action_close))
                    }
                }
                Text(
                    text = stringResource(R.string.insights_status_editor_description, scopeLabel),
                    style = MaterialTheme.typography.bodySmall,
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )
                if (isLoading) {
                    Text(stringResource(R.string.insights_status_editor_loading, scopeLabel))
                } else {
                    Column(
                        modifier = Modifier
                            .weight(1f)
                            .verticalScroll(rememberScrollState()),
                        verticalArrangement = Arrangement.spacedBy(8.dp)
                    ) {
                        config.statuses.forEach { definition ->
                            val status = statusValues.firstOrNull { it.id == definition.id }
                            Card(modifier = Modifier.fillMaxWidth()) {
                                Row(
                                    modifier = Modifier.fillMaxWidth().padding(12.dp),
                                    verticalAlignment = Alignment.CenterVertically
                                ) {
                                    Column(modifier = Modifier.weight(1f)) {
                                        Text(definition.label, style = MaterialTheme.typography.titleSmall)
                                        Text(
                                            text = stringResource(R.string.insights_daily_status_parent, definition.parent),
                                            style = MaterialTheme.typography.bodySmall
                                        )
                                    }
                                    Text(
                                        text = stringResource(
                                            R.string.insights_status_statistics,
                                            status?.occurrenceCount ?: 0,
                                            formatStatusDuration(status?.totalDurationSeconds ?: 0L)
                                        ),
                                        style = MaterialTheme.typography.bodySmall
                                    )
                                    IconButton(onClick = {
                                        editingId = definition.id
                                        selectedParentOverride = null
                                    }) {
                                        Icon(Icons.Default.Edit, contentDescription = stringResource(R.string.insights_daily_status_edit))
                                    }
                                    IconButton(onClick = {
                                        persistConfig(
                                            config.copy(
                                                statuses = config.statuses.filterNot {
                                                    it.id == definition.id
                                                }
                                            )
                                        )
                                    }) {
                                        Icon(Icons.Default.Delete, contentDescription = stringResource(R.string.insights_daily_status_delete))
                                    }
                                }
                            }
                        }
                        if (config.statuses.isEmpty()) {
                            Text(stringResource(R.string.insights_status_editor_empty, scopeLabel))
                        }
                        OutlinedButton(
                            onClick = {
                                val id = nextStatusId(config.statuses)
                                config = config.copy(
                                    statuses = config.statuses + DailyStatusDefinition(id, "", "")
                                )
                                editingId = id
                                selectedParentOverride = null
                            },
                            modifier = Modifier.fillMaxWidth()
                        ) {
                            Icon(Icons.Default.Add, contentDescription = null)
                            Text(stringResource(R.string.insights_daily_status_add))
                        }
                    }
                }
                if (errorMessage.isNotBlank()) {
                    Text(errorMessage, color = MaterialTheme.colorScheme.error)
                }
                if (isSaving) {
                    Text(
                        text = stringResource(R.string.insights_daily_status_saving),
                        style = MaterialTheme.typography.bodySmall,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                }
            }
        }
    }

    val editing = editingId?.let { id -> config.statuses.firstOrNull { it.id == id } }
    if (editing != null) {
        DailyStatusDefinitionDialog(
            initial = editing,
            scopeLabel = scopeLabel,
            parentOverride = selectedParentOverride,
            onFinish = { next ->
                if (next != null) {
                    updateStatus(next)
                } else if (editing.label.isBlank()) {
                    config = config.copy(statuses = config.statuses.filterNot { it.id == editing.id })
                    editingId = null
                } else {
                    editingId = null
                }
            },
            onChooseParent = {
                parentPickerStatusId = editing.id
                recordViewModel.openDailyStatusParentCatalog()
            }
        )
    }

    if (parentPickerStatusId != null && recordUiState.isCanonicalCatalogVisible) {
        RecordCanonicalCatalogScreen(
            isLoading = recordUiState.isCanonicalCatalogLoading,
            roots = recordUiState.canonicalCatalogRoots,
            statusText = recordUiState.canonicalCatalogStatusText,
            displayMode = RecordFrequentOutputMode.CANONICAL,
            target = CanonicalBrowserTarget.INSIGHTS_STATUS_PARENT,
            collapsedRootPaths = recordUiState.collapsedCanonicalRootPaths,
            orderedRootPaths = recordUiState.orderedCanonicalRootPaths,
            onDismissRequest = {
                parentPickerStatusId = null
                recordViewModel.dismissCanonicalCatalog()
            },
            onDisplayModeChange = {},
            onCollapsedRootPathsChange = recordViewModel::updateCollapsedCanonicalRootPaths,
            onOrderedRootPathsChange = recordViewModel::updateOrderedCanonicalRootPaths,
            onCanonicalEntryClick = {},
            onCanonicalParentClick = { path ->
                val id = parentPickerStatusId
                if (id != null) {
                    selectedParentOverride = path
                    config = config.copy(statuses = config.statuses.map {
                        if (it.id == id) it.copy(parent = path) else it
                    })
                }
                parentPickerStatusId = null
                recordViewModel.dismissCanonicalCatalog()
            }
        )
    }
}

@Composable
private fun DailyStatusDefinitionDialog(
    initial: DailyStatusDefinition,
    scopeLabel: String,
    parentOverride: String?,
    onFinish: (DailyStatusDefinition?) -> Unit,
    onChooseParent: () -> Unit
) {
    var label by remember(initial.id) { mutableStateOf(initial.label) }
    var parent by remember(initial.id) { mutableStateOf(initial.parent) }
    LaunchedEffect(parentOverride) {
        if (!parentOverride.isNullOrBlank()) {
            parent = parentOverride
            label = parentOverride
        }
    }
    fun finish() {
        onFinish(
            if (label.isNotBlank() && parent.isNotBlank()) {
                val normalizedParent = parent.trim()
                DailyStatusDefinition(
                    DailyStatusConfigStore.idForParent(normalizedParent),
                    label.trim(),
                    normalizedParent
                )
            } else {
                null
            }
        )
    }

    Dialog(onDismissRequest = ::finish) {
        Card(modifier = Modifier.fillMaxWidth().padding(24.dp)) {
            Column(
                modifier = Modifier.padding(20.dp),
                verticalArrangement = Arrangement.spacedBy(10.dp)
            ) {
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    verticalAlignment = Alignment.CenterVertically,
                    horizontalArrangement = Arrangement.SpaceBetween
                ) {
                    Text(
                        stringResource(R.string.insights_status_editor_edit_title, scopeLabel),
                        style = MaterialTheme.typography.titleMedium
                    )
                    IconButton(onClick = ::finish) {
                        Icon(
                            Icons.Default.Close,
                            contentDescription = stringResource(R.string.daily_status_action_close)
                        )
                    }
                }
                OutlinedTextField(label, { label = it }, label = { Text("Label") }, singleLine = true)
                OutlinedTextField(
                    value = parent,
                    onValueChange = {},
                    readOnly = true,
                    label = { Text("Parent") },
                    singleLine = true
                )
                OutlinedButton(onClick = onChooseParent, modifier = Modifier.fillMaxWidth()) {
                    Text(stringResource(R.string.insights_daily_status_choose_parent))
                }
            }
        }
    }
}

@Composable
private fun formatStatusDuration(totalSeconds: Long): String {
    val hours = totalSeconds / 3_600
    val minutes = (totalSeconds % 3_600) / 60
    val seconds = totalSeconds % 60
    return when {
        hours > 0 -> stringResource(R.string.insights_status_duration_hours_minutes, hours, minutes)
        minutes > 0 -> stringResource(R.string.insights_status_duration_minutes, minutes)
        else -> stringResource(R.string.insights_status_duration_seconds, seconds)
    }
}

private fun InsightsMode.statusEditorScopeResId(): Int = when (this) {
    InsightsMode.DAY -> R.string.insights_status_scope_day
    InsightsMode.WEEK -> R.string.insights_status_scope_week
    InsightsMode.MONTH -> R.string.insights_status_scope_month
    InsightsMode.YEAR -> R.string.insights_status_scope_year
    InsightsMode.RANGE -> R.string.insights_status_scope_range
    InsightsMode.RECENT -> R.string.insights_status_scope_recent
}

private fun nextStatusId(statuses: List<DailyStatusDefinition>): String {
    var index = statuses.size + 1
    var candidate: String
    do {
        candidate = "status_$index"
        index++
    } while (statuses.any { it.id == candidate })
    return candidate
}
