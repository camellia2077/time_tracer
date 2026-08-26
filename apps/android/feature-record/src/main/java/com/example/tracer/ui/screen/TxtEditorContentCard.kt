package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.MoreVert
import androidx.compose.material3.ElevatedCard
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.ModalBottomSheet
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.material3.rememberModalBottomSheetState
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.record.R
import java.time.LocalDate


@Composable
internal fun TxtEditorContentCard(
    selectedHistoryFile: String,
    currentDay: LocalDate?,
    onConvertActivityNames: (TxtActivityNameTargetMode) -> Unit,
    dayBlockEditorState: TxtDayBlockResolveResult,
    inlineStatusText: String,
    onCanonicalCatalogRequested: () -> Unit = {},
    onReloadTxtData: () -> Unit,
    onOpenRawEditor: (TxtOutputMode) -> Unit,
    structuredDayEdit: TxtDayEditResolveResult? = null,
    canonicalCatalogRoots: List<CanonicalPathNode> = emptyList(),
    isCanonicalCatalogLoading: Boolean = false,
    canonicalCatalogStatusText: String = "",
    collapsedCanonicalRootPaths: Set<String> = emptySet(),
    orderedCanonicalRootPaths: List<String> = emptyList(),
    onCollapsedCanonicalRootPathsChange: (Set<String>) -> Unit = {},
    onOrderedCanonicalRootPathsChange: (List<String>) -> Unit = {},
    onStructuredDayEditApply: (String, List<TxtDayEditEvent>) -> Unit = { _, _ -> },
    onStructuredDayActivityReplace: suspend (
        String,
        String,
        String,
        List<TxtDayEditEvent>
    ) -> TxtDayActivityReplacementResult = { _, _, _, _ ->
        TxtDayActivityReplacementResult(
            ok = false,
            replacedEventCount = 0,
            message = "TXT activity replacement is unavailable."
        )
    },
    onPrepareMonthActivityEdits: suspend () -> TxtMonthActivityEditsResult = {
        TxtMonthActivityEditsResult(false, null, "TXT activity replacement is unavailable.")
    },
    onReplaceMonthActivity: suspend (
        TxtMonthActivityEditSnapshot,
        String,
        String
    ) -> TxtMonthActivityReplacementResult = { _, _, _ ->
        TxtMonthActivityReplacementResult(false, 0, "TXT activity replacement is unavailable.")
    }
) {
    val canShowStructuredDay =
        structuredDayEdit?.ok == true && structuredDayEdit.found && structuredDayEdit.canSave
    var findReplaceVisible by remember(selectedHistoryFile) {
        mutableStateOf(false)
    }
    var moreEditsVisible by remember(selectedHistoryFile) {
        mutableStateOf(false)
    }
    var activityNameToolsVisible by remember(selectedHistoryFile) {
        mutableStateOf(false)
    }
    var pendingActivityNameConversion by remember(selectedHistoryFile) {
        mutableStateOf<TxtActivityNameTargetMode?>(null)
    }
    val currentDayText = currentDay?.let { formatEditorCurrentDayText(it) }
    val isStructuredDayLoading = dayBlockEditorState.canSave && structuredDayEdit == null
    ElevatedCard(modifier = Modifier.fillMaxWidth()) {
        Column(
            modifier = Modifier.padding(16.dp),
            verticalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            Text(
                text = stringResource(R.string.txt_label_editor_file, selectedHistoryFile),
                style = MaterialTheme.typography.titleMedium,
                color = MaterialTheme.colorScheme.primary
            )

            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.End
            ) {
                IconButton(onClick = { moreEditsVisible = true }) {
                    Icon(
                        imageVector = Icons.Default.MoreVert,
                        contentDescription = stringResource(R.string.txt_editor_more_edits)
                    )
                }
            }

            if (inlineStatusText.isNotBlank()) {
                val isError = inlineStatusText.contains("fail", ignoreCase = true) ||
                    inlineStatusText.contains("error", ignoreCase = true) ||
                    inlineStatusText.contains("invalid", ignoreCase = true) ||
                    inlineStatusText.contains("blocked", ignoreCase = true) ||
                    inlineStatusText.contains("duplicate", ignoreCase = true) ||
                    inlineStatusText.contains("mismatch", ignoreCase = true) ||
                    inlineStatusText.contains("missing", ignoreCase = true)
                val statusColor = if (isError) {
                    MaterialTheme.colorScheme.error
                } else {
                    MaterialTheme.colorScheme.primary
                }
                Text(
                    text = inlineStatusText,
                    style = MaterialTheme.typography.bodySmall,
                    color = statusColor,
                    modifier = Modifier.fillMaxWidth()
                )
            }

            if (currentDayText != null) {
                Text(
                    text = currentDayText,
                    style = MaterialTheme.typography.bodyMedium,
                    color = MaterialTheme.colorScheme.onSurface,
                    maxLines = 1,
                    overflow = TextOverflow.Ellipsis,
                    modifier = Modifier.fillMaxWidth()
                )
            }

            if (isStructuredDayLoading) {
                TxtStructuredDayLoading()
            } else if (canShowStructuredDay) {
                TxtStructuredDayEditor(
                    result = structuredDayEdit,
                    roots = canonicalCatalogRoots,
                    catalogLoading = isCanonicalCatalogLoading,
                    catalogStatusText = canonicalCatalogStatusText,
                    collapsedRootPaths = collapsedCanonicalRootPaths,
                    orderedRootPaths = orderedCanonicalRootPaths,
                    onCollapsedRootPathsChange = onCollapsedCanonicalRootPathsChange,
                    onOrderedRootPathsChange = onOrderedCanonicalRootPathsChange,
                    onApply = onStructuredDayEditApply
                )
            }
            TxtActivityFindReplace(
                visible = findReplaceVisible,
                events = structuredDayEdit?.events.orEmpty(),
                dayRemark = structuredDayEdit?.dayRemark.orEmpty(),
                roots = canonicalCatalogRoots,
                catalogLoading = isCanonicalCatalogLoading,
                catalogStatusText = canonicalCatalogStatusText,
                collapsedRootPaths = collapsedCanonicalRootPaths,
                orderedRootPaths = orderedCanonicalRootPaths,
                onCollapsedRootPathsChange = onCollapsedCanonicalRootPathsChange,
                onOrderedRootPathsChange = onOrderedCanonicalRootPathsChange,
                onCanonicalCatalogRequested = onCanonicalCatalogRequested,
                onDismiss = { findReplaceVisible = false },
                onReplaceDayActivity = onStructuredDayActivityReplace,
                onLoadMonthActivities = onPrepareMonthActivityEdits,
                onReplaceMonthActivity = onReplaceMonthActivity
            )
            if (moreEditsVisible) {
                TxtMoreEditsSheet(
                    onDismiss = { moreEditsVisible = false },
                    onOpenRawEditor = {
                        moreEditsVisible = false
                        onOpenRawEditor(TxtOutputMode.DAY)
                    },
                    onOpenMonthFindReplace = {
                        moreEditsVisible = false
                        findReplaceVisible = true
                    },
                    onReloadTxtData = {
                        moreEditsVisible = false
                        onReloadTxtData()
                    },
                    onOpenActivityNameTools = {
                        moreEditsVisible = false
                        activityNameToolsVisible = true
                    }
                )
            }
            if (activityNameToolsVisible) {
                TxtActivityNameToolsSheet(
                    onDismiss = { activityNameToolsVisible = false },
                    onSelectTargetMode = { targetMode ->
                        activityNameToolsVisible = false
                        pendingActivityNameConversion = targetMode
                    }
                )
            }
            pendingActivityNameConversion?.let { targetMode ->
                TxtActivityNameConversionConfirmDialog(
                    targetMode = targetMode,
                    onDismiss = { pendingActivityNameConversion = null },
                    onConfirm = {
                        pendingActivityNameConversion = null
                        onConvertActivityNames(targetMode)
                    }
                )
            }
        }
    }
}

@Composable
@OptIn(ExperimentalMaterial3Api::class)
private fun TxtMoreEditsSheet(
    onDismiss: () -> Unit,
    onOpenRawEditor: () -> Unit,
    onOpenMonthFindReplace: () -> Unit,
    onReloadTxtData: () -> Unit,
    onOpenActivityNameTools: () -> Unit
) {
    val sheetState = rememberModalBottomSheetState(skipPartiallyExpanded = true)
    ModalBottomSheet(onDismissRequest = onDismiss, sheetState = sheetState) {
        Column(
            modifier = Modifier.padding(start = 24.dp, top = 8.dp, end = 24.dp, bottom = 24.dp),
            verticalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            Text(
                text = stringResource(R.string.txt_editor_more_edits),
                style = MaterialTheme.typography.headlineSmall
            )
            TextButton(
                modifier = Modifier.fillMaxWidth(),
                onClick = onOpenMonthFindReplace
            ) {
                Text(stringResource(R.string.txt_month_edit_find_replace))
            }
            TextButton(
                modifier = Modifier.fillMaxWidth(),
                onClick = onReloadTxtData
            ) {
                Text(stringResource(R.string.txt_editor_reload_txt_data))
            }
            TextButton(
                modifier = Modifier.fillMaxWidth(),
                onClick = onOpenActivityNameTools
            ) {
                Text(stringResource(R.string.txt_month_activity_name_tools))
            }
            TextButton(
                modifier = Modifier.fillMaxWidth(),
                onClick = onOpenRawEditor
            ) {
                Text(stringResource(R.string.txt_raw_editor_open))
            }
        }
    }
}

@Composable
@OptIn(ExperimentalMaterial3Api::class)
private fun TxtActivityNameToolsSheet(
    onDismiss: () -> Unit,
    onSelectTargetMode: (TxtActivityNameTargetMode) -> Unit
) {
    val sheetState = rememberModalBottomSheetState(skipPartiallyExpanded = true)
    ModalBottomSheet(onDismissRequest = onDismiss, sheetState = sheetState) {
        Column(
            modifier = Modifier.padding(start = 24.dp, top = 8.dp, end = 24.dp, bottom = 24.dp),
            verticalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            Text(
                text = stringResource(R.string.txt_month_activity_name_tools),
                style = MaterialTheme.typography.headlineSmall
            )
            Text(
                text = stringResource(R.string.txt_month_activity_name_tools_description),
                color = MaterialTheme.colorScheme.onSurfaceVariant
            )
            TextButton(
                modifier = Modifier.fillMaxWidth(),
                onClick = { onSelectTargetMode(TxtActivityNameTargetMode.CANONICAL) }
            ) {
                Text(stringResource(R.string.txt_month_convert_aliases_to_canonical))
            }
            TextButton(
                modifier = Modifier.fillMaxWidth(),
                onClick = { onSelectTargetMode(TxtActivityNameTargetMode.ALIAS) }
            ) {
                Text(stringResource(R.string.txt_month_convert_canonical_to_aliases))
            }
        }
    }
}

@Composable
private fun TxtActivityNameConversionConfirmDialog(
    targetMode: TxtActivityNameTargetMode,
    onDismiss: () -> Unit,
    onConfirm: () -> Unit
) {
    val conversionName = stringResource(
        when (targetMode) {
            TxtActivityNameTargetMode.CANONICAL ->
                R.string.txt_month_convert_aliases_to_canonical
            TxtActivityNameTargetMode.ALIAS ->
                R.string.txt_month_convert_canonical_to_aliases
        }
    )
    androidx.compose.material3.AlertDialog(
        onDismissRequest = onDismiss,
        title = { Text(stringResource(R.string.txt_month_activity_name_convert_confirm_title)) },
        text = {
            Text(
                stringResource(
                    R.string.txt_month_activity_name_convert_confirm_message,
                    conversionName
                )
            )
        },
        confirmButton = {
            TextButton(onClick = onConfirm) {
                Text(stringResource(R.string.txt_month_convert_action))
            }
        },
        dismissButton = {
            TextButton(onClick = onDismiss) {
                Text(stringResource(R.string.txt_action_close))
            }
        }
    )
}

@Composable
private fun TxtStructuredDayLoading() {
    Column(
        modifier = Modifier
            .fillMaxWidth()
            .padding(vertical = 24.dp),
        horizontalAlignment = Alignment.CenterHorizontally,
        verticalArrangement = Arrangement.spacedBy(12.dp)
    ) {
        CircularProgressIndicator()
        Text(
            text = stringResource(R.string.record_hint_loading),
            style = MaterialTheme.typography.bodyMedium,
            color = MaterialTheme.colorScheme.onSurfaceVariant
        )
    }
}
