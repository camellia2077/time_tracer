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
internal fun RecordCanonicalCatalogSection(
    isLoading: Boolean,
    roots: List<CanonicalPathNode>,
    statusText: String,
    displayMode: RecordSuggestionOutputMode,
    target: CanonicalBrowserTarget? = null,
    collapsedRootPaths: Set<String>,
    orderedRootPaths: List<String>,
    onCollapsedRootPathsChange: (Set<String>) -> Unit,
    onOrderedRootPathsChange: (List<String>) -> Unit,
    onCanonicalEntryClick: (CanonicalCatalogEntry) -> Unit,
    onCanonicalParentClick: (String) -> Unit,
    modifier: Modifier = Modifier
) {
    val parentSelectionEnabled = target == CanonicalBrowserTarget.INSIGHTS_STATUS_PARENT
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
                } else if (target == CanonicalBrowserTarget.INSIGHTS_STATUS_PARENT) {
                    R.string.record_canonical_catalog_parent_title
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
                } else if (target == CanonicalBrowserTarget.INSIGHTS_STATUS_PARENT) {
                    R.string.record_canonical_catalog_parent_description
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
                        parentSelectionEnabled = parentSelectionEnabled,
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
                        onCanonicalEntryClick = onCanonicalEntryClick,
                        onCanonicalParentClick = onCanonicalParentClick
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
internal fun CanonicalRootDropPreview(modifier: Modifier = Modifier) {
    HorizontalDivider(
        modifier = modifier
            .fillMaxWidth()
            .padding(horizontal = 4.dp)
            .testTag(canonicalCatalogRootDropPreviewTestTag()),
        thickness = 3.dp,
        color = MaterialTheme.colorScheme.primary
    )
}

