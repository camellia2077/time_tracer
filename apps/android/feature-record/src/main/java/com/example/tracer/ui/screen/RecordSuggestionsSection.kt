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

@Composable
private fun RecordCanonicalCatalogSection(
    isLoading: Boolean,
    roots: List<CanonicalPathNode>,
    statusText: String,
    displayMode: RecordSuggestionOutputMode,
    target: CanonicalBrowserTarget? = null,
    collapsedRootPaths: Set<String>,
    orderedRootPaths: List<String>,
    onCollapsedRootPathsChange: (Set<String>) -> Unit,
    onOrderedRootPathsChange: (List<String>) -> Unit,
    onCanonicalPathClick: (String) -> Unit,
    modifier: Modifier = Modifier
) {
    val orderedRoots = remember(roots, orderedRootPaths) {
        roots.orderCanonicalRoots(orderedRootPaths)
    }
    val orderedRootPathKey = remember(orderedRoots) {
        orderedRoots.joinToString(separator = "\u001F") { it.path }
    }
    var draggedRootPath by remember { mutableStateOf<String?>(null) }
    var draggedRootStartIndex by remember { mutableIntStateOf(-1) }
    var dropTargetIndex by remember { mutableIntStateOf(-1) }
    var draggedPointerCenterY by remember { mutableStateOf<Float?>(null) }
    val rootMetricsPx = remember { mutableStateMapOf<String, RootDragMetrics>() }
    val hapticFeedback = LocalHapticFeedback.current

    LaunchedEffect(orderedRoots) {
        rootMetricsPx.keys
            .filter { path -> orderedRoots.none { it.path == path } }
            .toList()
            .forEach(rootMetricsPx::remove)
    }

    fun clearDragState() {
        draggedRootPath = null
        draggedRootStartIndex = -1
        dropTargetIndex = -1
        draggedPointerCenterY = null
    }

    fun cancelDrag() {
        clearDragState()
    }

    fun finishDrag() {
        val startIndex = draggedRootStartIndex
        val targetIndex = dropTargetIndex
        val canReorder = startIndex in orderedRoots.indices &&
            targetIndex in orderedRoots.indices &&
            startIndex != targetIndex
        val reorderedPaths = if (canReorder) {
            orderedRoots.moveItem(startIndex, targetIndex).map(CanonicalPathNode::path)
        } else {
            orderedRoots.map(CanonicalPathNode::path)
        }
        clearDragState()
        if (!canReorder) {
            return
        }
        onOrderedRootPathsChange(reorderedPaths)
    }

    fun showDropPreviewBefore(index: Int): Boolean =
        draggedRootPath != null &&
            dropTargetIndex != draggedRootStartIndex &&
            dropTargetIndex < draggedRootStartIndex &&
            index == dropTargetIndex

    fun showDropPreviewAfter(index: Int): Boolean =
        draggedRootPath != null &&
            dropTargetIndex != draggedRootStartIndex &&
            dropTargetIndex > draggedRootStartIndex &&
            index == dropTargetIndex

    fun updateDropTarget(pointerCenterY: Float) {
        val targetIndex = calculateCanonicalRootDropIndex(
            pointerCenterY = pointerCenterY,
            roots = orderedRoots,
            rootMetricsPx = rootMetricsPx
        )
        if (targetIndex >= 0) {
            dropTargetIndex = targetIndex
        }
    }

    Column(
        modifier = modifier,
        verticalArrangement = Arrangement.spacedBy(8.dp)
    ) {
        Text(
            text = stringResource(
                if (target == CanonicalBrowserTarget.QUICK_ACCESS) {
                    R.string.record_canonical_catalog_quick_access_title
                } else {
                    R.string.record_canonical_catalog_title
                }
            ),
            style = MaterialTheme.typography.titleMedium,
            color = MaterialTheme.colorScheme.primary
        )
        Text(
            text = stringResource(
                if (target == CanonicalBrowserTarget.QUICK_ACCESS) {
                    R.string.record_canonical_catalog_quick_access_description
                } else {
                    R.string.record_canonical_catalog_description
                }
            ),
            style = MaterialTheme.typography.bodySmall,
            color = MaterialTheme.colorScheme.onSurfaceVariant
        )

        when {
            isLoading -> {
                Text(
                    text = stringResource(R.string.record_canonical_catalog_loading),
                    style = MaterialTheme.typography.bodySmall
                )
            }

            orderedRoots.isNotEmpty() -> {
                orderedRoots.forEachIndexed { index, root ->
                    val isDragging = draggedRootPath == root.path
                    val rootMetrics = rootMetricsPx[root.path]
                    val dragTranslationY =
                        if (isDragging && rootMetrics != null && draggedPointerCenterY != null) {
                            draggedPointerCenterY!! - rootMetrics.centerYPx
                        } else {
                            0f
                        }
                    if (showDropPreviewBefore(index)) {
                        CanonicalRootDropPreview()
                    }
                    CanonicalPathNodeCard(
                        node = root,
                        displayMode = displayMode,
                        collapsedRootPaths = collapsedRootPaths,
                        onCollapsedRootPathsChange = onCollapsedRootPathsChange,
                        modifier = Modifier
                            .fillMaxWidth()
                            .testTag(canonicalCatalogRootTestTag(root.path))
                            .onGloballyPositioned { coordinates ->
                                rootMetricsPx[root.path] = RootDragMetrics(
                                    topPx = coordinates.positionInParent().y,
                                    heightPx = coordinates.size.height
                                )
                            }
                            .zIndex(if (isDragging) 1f else 0f)
                            .graphicsLayer {
                                if (isDragging) {
                                    translationY = dragTranslationY
                                    shadowElevation = 12.dp.toPx()
                                    scaleX = 1.01f
                                    scaleY = 1.01f
                                }
                            }
                            .pointerInput(root.path, orderedRootPathKey) {
                                detectDragGesturesAfterLongPress(
                                    onDragStart = {
                                        hapticFeedback.performHapticFeedback(
                                            HapticFeedbackType.LongPress
                                        )
                                        draggedRootPath = root.path
                                        draggedRootStartIndex =
                                            orderedRoots.indexOfFirst { it.path == root.path }
                                        dropTargetIndex = draggedRootStartIndex
                                        draggedPointerCenterY =
                                            rootMetricsPx[root.path]?.centerYPx
                                    },
                                    onDragCancel = ::cancelDrag,
                                    onDragEnd = ::finishDrag,
                                    onDrag = { change, dragAmount ->
                                        if (
                                            draggedRootPath != root.path ||
                                            draggedRootStartIndex !in orderedRoots.indices
                                        ) {
                                            return@detectDragGesturesAfterLongPress
                                        }
                                        change.consume()
                                        val currentMetrics = rootMetricsPx[root.path]
                                            ?: return@detectDragGesturesAfterLongPress
                                        val pointerCenterY =
                                            (draggedPointerCenterY ?: currentMetrics.centerYPx) +
                                                dragAmount.y
                                        draggedPointerCenterY = pointerCenterY
                                        updateDropTarget(pointerCenterY)
                                    }
                                )
                            },
                        onCanonicalPathClick = onCanonicalPathClick
                    )
                    if (showDropPreviewAfter(index)) {
                        CanonicalRootDropPreview()
                    }
                }
            }

            statusText.isNotBlank() -> {
                Text(
                    text = statusText,
                    style = MaterialTheme.typography.bodySmall,
                    color = MaterialTheme.colorScheme.error
                )
            }

            else -> {
                Text(
                    text = stringResource(R.string.record_canonical_catalog_empty),
                    style = MaterialTheme.typography.bodySmall
                )
            }
        }
    }
}

@Composable
private fun CanonicalRootDropPreview(modifier: Modifier = Modifier) {
    HorizontalDivider(
        modifier = modifier
            .fillMaxWidth()
            .padding(horizontal = 4.dp)
            .testTag(canonicalCatalogRootDropPreviewTestTag()),
        thickness = 3.dp,
        color = MaterialTheme.colorScheme.primary
    )
}

@Composable
internal fun RecordCanonicalCatalogScreen(
    isLoading: Boolean,
    roots: List<CanonicalPathNode>,
    statusText: String,
    displayMode: RecordSuggestionOutputMode,
    target: CanonicalBrowserTarget? = null,
    collapsedRootPaths: Set<String>,
    orderedRootPaths: List<String>,
    onDismissRequest: () -> Unit,
    onDisplayModeChange: (RecordSuggestionOutputMode) -> Unit,
    onCollapsedRootPathsChange: (Set<String>) -> Unit,
    onOrderedRootPathsChange: (List<String>) -> Unit,
    onCanonicalPathClick: (String) -> Unit
) {
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
                            text = stringResource(
                                if (target == CanonicalBrowserTarget.QUICK_ACCESS) {
                                    R.string.record_canonical_catalog_quick_access_title
                                } else {
                                    R.string.record_canonical_catalog_title
                                }
                            ),
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
                        if (displayMode == RecordSuggestionOutputMode.CANONICAL) {
                            FilledIconButton(onClick = {}) {
                                Icon(
                                    imageVector = Icons.Default.Code,
                                    contentDescription = stringResource(
                                        R.string.record_cd_canonical_catalog_display_canonical_selected
                                    )
                                )
                            }
                        } else {
                            OutlinedIconButton(
                                onClick = {
                                    onDisplayModeChange(RecordSuggestionOutputMode.CANONICAL)
                                }
                            ) {
                                Icon(
                                    imageVector = Icons.Default.Code,
                                    contentDescription = stringResource(
                                        R.string.record_cd_canonical_catalog_display_switch_to_canonical
                                    )
                                )
                            }
                        }
                        if (displayMode == RecordSuggestionOutputMode.ALIAS) {
                            FilledIconButton(onClick = {}) {
                                Icon(
                                    imageVector = Icons.Default.Translate,
                                    contentDescription = stringResource(
                                        R.string.record_cd_canonical_catalog_display_alias_selected
                                    )
                                )
                            }
                        } else {
                            OutlinedIconButton(
                                onClick = {
                                    onDisplayModeChange(RecordSuggestionOutputMode.ALIAS)
                                }
                            ) {
                                Icon(
                                    imageVector = Icons.Default.Translate,
                                    contentDescription = stringResource(
                                        R.string.record_cd_canonical_catalog_display_switch_to_alias
                                    )
                                )
                            }
                        }
                    }
                    Text(
                        text = stringResource(
                            if (displayMode == RecordSuggestionOutputMode.CANONICAL) {
                                R.string.record_canonical_catalog_display_mode_canonical
                            } else {
                                R.string.record_canonical_catalog_display_mode_alias
                            }
                        ),
                        style = MaterialTheme.typography.bodyMedium,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                }
                HorizontalDivider()
                Box(
                    modifier = Modifier
                        .weight(1f)
                        .fillMaxWidth()
                        .verticalScroll(rememberScrollState())
                ) {
                    RecordCanonicalCatalogSection(
                        isLoading = isLoading,
                        roots = roots,
                        statusText = statusText,
                        displayMode = displayMode,
                        target = target,
                        collapsedRootPaths = collapsedRootPaths,
                        orderedRootPaths = orderedRootPaths,
                        onCollapsedRootPathsChange = onCollapsedRootPathsChange,
                        onOrderedRootPathsChange = onOrderedRootPathsChange,
                        onCanonicalPathClick = onCanonicalPathClick,
                        modifier = Modifier
                            .fillMaxWidth()
                            .padding(start = 24.dp, end = 24.dp, top = 16.dp, bottom = 24.dp)
                    )
                }
            }
        }
    }
}

@Composable
private fun CanonicalPathNodeCard(
    node: CanonicalPathNode,
    displayMode: RecordSuggestionOutputMode,
    collapsedRootPaths: Set<String>,
    onCollapsedRootPathsChange: (Set<String>) -> Unit,
    onCanonicalPathClick: (String) -> Unit,
    modifier: Modifier = Modifier,
    depth: Int = 0
) {
    val hasCollapsibleContent = node.entries.isNotEmpty() || node.children.isNotEmpty()
    val expanded = !collapsedRootPaths.contains(node.path)
    val isRootNode = depth == 0
    val cardBorder = BorderStroke(
        width = 1.dp,
        color = if (isRootNode) {
            MaterialTheme.colorScheme.primary
        } else {
            MaterialTheme.colorScheme.primary.copy(alpha = 0.32f)
        }
    )

    OutlinedCard(
        modifier = modifier.fillMaxWidth(),
        border = cardBorder
    ) {
        Column(
            modifier = Modifier.padding(12.dp),
            verticalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            Row(
                modifier = Modifier.fillMaxWidth(),
                verticalAlignment = Alignment.CenterVertically,
                horizontalArrangement = Arrangement.SpaceBetween
            ) {
                Text(
                    text = node.displayTitle(depth),
                    modifier = Modifier.weight(1f),
                    style = if (isRootNode) {
                        MaterialTheme.typography.titleMedium
                    } else {
                        MaterialTheme.typography.bodyMedium
                    },
                    fontWeight = FontWeight.SemiBold,
                    color = if (isRootNode) {
                        MaterialTheme.colorScheme.primary
                    } else {
                        MaterialTheme.colorScheme.onSurface
                    }
                )
                if (hasCollapsibleContent) {
                    IconButton(
                        onClick = {
                            val next = collapsedRootPaths.toMutableSet()
                            if (expanded) {
                                next += node.path
                            } else {
                                next -= node.path
                            }
                            onCollapsedRootPathsChange(next)
                        }
                    ) {
                        Icon(
                            imageVector = if (expanded) {
                                Icons.Default.ExpandLess
                            } else {
                                Icons.Default.ExpandMore
                            },
                            contentDescription = null
                        )
                    }
                }
            }
            if (expanded && node.entries.isNotEmpty()) {
                com.example.tracer.ui.components.SimpleFlowRow(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalGap = 8.dp,
                    verticalGap = 8.dp
                ) {
                    node.entries.forEach { entry ->
                        val displayToken = entry.displayToken(displayMode)
                        val displayLabel = entry.displayLabel(displayMode)
                        SuggestionChip(
                            onClick = { onCanonicalPathClick(displayToken) },
                            label = {
                                Text(
                                    text = displayLabel,
                                    maxLines = 1,
                                    overflow = TextOverflow.Ellipsis
                                )
                            }
                        )
                    }
                }
            }
            if (expanded) {
                node.children.forEach { child ->
                    CanonicalPathNodeCard(
                        node = child,
                        displayMode = displayMode,
                        collapsedRootPaths = collapsedRootPaths,
                        onCollapsedRootPathsChange = onCollapsedRootPathsChange,
                        onCanonicalPathClick = onCanonicalPathClick,
                        modifier = Modifier,
                        depth = depth + 1
                    )
                }
            }
        }
    }
}

private fun String.displayCanonicalPathForUi(): String = this

private fun CanonicalPathNode.displayTitle(depth: Int): String =
    if (depth == 0) {
        path.displayCanonicalPathForUi()
    } else {
        name.displayCanonicalPathForUi()
    }

private fun CanonicalCatalogEntry.displayToken(displayMode: RecordSuggestionOutputMode): String {
    val canonical = canonicalPath.trim()
    if (displayMode != RecordSuggestionOutputMode.ALIAS) {
        return canonical
    }
    return aliases.firstOrNull { alias -> alias.isNotBlank() }?.trim().orEmpty().ifEmpty { canonical }
}

private fun CanonicalCatalogEntry.displayLabel(displayMode: RecordSuggestionOutputMode): String {
    if (displayMode == RecordSuggestionOutputMode.ALIAS) {
        return displayToken(displayMode).displayCanonicalPathForUi()
    }
    return canonicalLeaf.trim().ifEmpty { canonicalPath.trim() }.displayCanonicalPathForUi()
}

private data class RootDragMetrics(
    val topPx: Float,
    val heightPx: Int
) {
    val centerYPx: Float
        get() = topPx + (heightPx / 2f)
}

private val CanonicalRootTagSanitizer = Regex("[^A-Za-z0-9]+")

private fun calculateCanonicalRootDropIndex(
    pointerCenterY: Float,
    roots: List<CanonicalPathNode>,
    rootMetricsPx: Map<String, RootDragMetrics>
): Int {
    if (roots.isEmpty()) {
        return -1
    }
    return roots.indices.minByOrNull { index ->
        val candidatePath = roots[index].path
        val candidateMetrics = rootMetricsPx[candidatePath]
        abs((candidateMetrics?.centerYPx ?: pointerCenterY) - pointerCenterY)
    } ?: -1
}

private fun List<CanonicalPathNode>.orderCanonicalRoots(orderedRootPaths: List<String>): List<CanonicalPathNode> {
    if (isEmpty()) {
        return this
    }
    if (orderedRootPaths.isEmpty()) {
        return this
    }
    val orderIndexByPath = orderedRootPaths.withIndex().associate { it.value to it.index }
    return withIndex()
        .sortedWith(
            compareBy<IndexedValue<CanonicalPathNode>> {
                orderIndexByPath[it.value.path] ?: Int.MAX_VALUE
            }.thenBy { it.index }
        )
        .map { it.value }
}

internal fun canonicalCatalogRootDropPreviewTestTag(): String =
    "canonical_catalog_root_drop_preview"

internal fun canonicalCatalogRootTestTag(rootPath: String): String {
    val normalized = rootPath
        .trim()
        .replace(CanonicalRootTagSanitizer, "_")
        .trim('_')
        .ifBlank { "blank" }
    return "canonical_catalog_root_$normalized"
}
