package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxHeight
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.navigationBarsPadding
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.widthIn
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.ArrowBack
import androidx.compose.material.icons.automirrored.filled.ArrowForward
import androidx.compose.material3.ElevatedCard
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.SegmentedButton
import androidx.compose.material3.SegmentedButtonDefaults
import androidx.compose.material3.SingleChoiceSegmentedButtonRow
import androidx.compose.material3.Text
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
import com.example.tracer.ui.components.TracerSegmentedButtonDefaults
import java.time.LocalDate


@Composable
internal fun TxtEditorContentCard(
    selectedHistoryFile: String,
    currentDay: LocalDate?,
    outputMode: TxtOutputMode,
    onOutputModeChange: (TxtOutputMode) -> Unit,
    activityNameTargetMode: TxtActivityNameTargetMode,
    onActivityNameTargetModeChange: (TxtActivityNameTargetMode) -> Unit,
    dayBlockEditorState: TxtDayBlockResolveResult,
    dayMarkerInput: String,
    inlineStatusText: String,
    editorText: String,
    hasUnsavedChanges: Boolean,
    canEditDay: Boolean,
    canIngest: Boolean,
    onEditorTextChange: (String) -> Unit,
    onIngest: () -> Unit,
    structuredDayEdit: TxtDayEditResolveResult? = null,
    canonicalCatalogRoots: List<CanonicalPathNode> = emptyList(),
    isCanonicalCatalogLoading: Boolean = false,
    canonicalCatalogStatusText: String = "",
    collapsedCanonicalRootPaths: Set<String> = emptySet(),
    orderedCanonicalRootPaths: List<String> = emptyList(),
    onCollapsedCanonicalRootPathsChange: (Set<String>) -> Unit = {},
    onOrderedCanonicalRootPathsChange: (List<String>) -> Unit = {},
    onStructuredDayEditApply: (String, List<TxtDayEditEvent>) -> Unit = { _, _ -> }
) {
    val canShowStructuredDay =
        structuredDayEdit?.ok == true && structuredDayEdit.found && structuredDayEdit.canSave
    var dayViewMode by remember(
        selectedHistoryFile,
        dayBlockEditorState.normalizedDayMarker
    ) {
        // The async Structured resolve is unavailable on the first frame. Keep the
        // default selection stable so that its arrival does not animate Raw -> Structured.
        mutableStateOf(TxtDayViewMode.STRUCTURED)
    }
    val dayContentIsoDate = dayBlockEditorState.dayContentIsoDate
    val currentDayText = currentDay?.let { formatEditorCurrentDayText(it) }
    val dayMarkerText = dayBlockEditorState.normalizedDayMarker.ifBlank { dayMarkerInput }
    val isStructuredDayLoading =
        outputMode == TxtOutputMode.DAY &&
            dayViewMode == TxtDayViewMode.STRUCTURED &&
            canEditDay &&
            structuredDayEdit == null
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

            val outputModes = TxtOutputMode.entries
            SingleChoiceSegmentedButtonRow(modifier = Modifier.fillMaxWidth()) {
                outputModes.forEachIndexed { index, mode ->
                    SegmentedButton(
                        shape = SegmentedButtonDefaults.itemShape(index = index, count = outputModes.size),
                        onClick = { onOutputModeChange(mode) },
                        selected = outputMode == mode,
                        colors = TracerSegmentedButtonDefaults.colors(),
                        modifier = Modifier.weight(1f),
                        label = {
                            Text(
                                text = when (mode) {
                                    TxtOutputMode.ALL -> stringResource(R.string.txt_mode_all)
                                    TxtOutputMode.DAY -> stringResource(R.string.txt_mode_day)
                                }
                            )
                        }
                    )
                }
            }
            if (outputMode == TxtOutputMode.DAY) {
                val dayViewModes = TxtDayViewMode.entries
                SingleChoiceSegmentedButtonRow(modifier = Modifier.fillMaxWidth()) {
                    dayViewModes.forEachIndexed { index, mode ->
                        SegmentedButton(
                            shape = SegmentedButtonDefaults.itemShape(
                                index = index,
                                count = dayViewModes.size
                            ),
                            onClick = { dayViewMode = mode },
                            selected = dayViewMode == mode,
                            enabled = mode != TxtDayViewMode.STRUCTURED ||
                                canShowStructuredDay ||
                                isStructuredDayLoading,
                            colors = TracerSegmentedButtonDefaults.colors(),
                            modifier = Modifier.weight(1f),
                            label = {
                                Text(
                                    when (mode) {
                                        TxtDayViewMode.STRUCTURED ->
                                            stringResource(R.string.txt_day_view_structured)
                                        TxtDayViewMode.RAW ->
                                            stringResource(R.string.txt_day_view_raw)
                                    }
                                )
                            }
                        )
                    }
                }
            }

            if (outputMode == TxtOutputMode.ALL) {
                val activityNameModes = TxtActivityNameTargetMode.entries
                SingleChoiceSegmentedButtonRow(modifier = Modifier.fillMaxWidth()) {
                    activityNameModes.forEachIndexed { index, mode ->
                        SegmentedButton(
                            shape = SegmentedButtonDefaults.itemShape(
                                index = index,
                                count = activityNameModes.size
                            ),
                            onClick = { onActivityNameTargetModeChange(mode) },
                            selected = activityNameTargetMode == mode,
                            colors = TracerSegmentedButtonDefaults.colors(),
                            modifier = Modifier.weight(1f),
                            label = {
                                Text(
                                    text = when (mode) {
                                        TxtActivityNameTargetMode.ALIAS ->
                                            stringResource(R.string.txt_mode_alias)
                                        TxtActivityNameTargetMode.CANONICAL ->
                                            stringResource(R.string.txt_mode_canonical)
                                    }
                                )
                            }
                        )
                    }
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
            } else if (
                outputMode == TxtOutputMode.DAY &&
                    dayViewMode == TxtDayViewMode.STRUCTURED &&
                    canShowStructuredDay
            ) {
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
            } else {
                TxtEditorInlineContent(
                    value = editorText,
                    outputMode = outputMode,
                    currentDayText = currentDayText,
                    dayMarkerText = dayMarkerText,
                    dayContentIsoDate = dayContentIsoDate,
                    hasUnsavedChanges = hasUnsavedChanges,
                    canEditDay = canEditDay,
                    canIngest = canIngest,
                    onEditorTextChange = onEditorTextChange,
                    onIngest = onIngest
                )
            }
        }
    }
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

private enum class TxtDayViewMode {
    STRUCTURED,
    RAW
}
