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
internal fun CanonicalPathNodeCard(
    node: CanonicalPathNode,
    displayMode: RecordSuggestionOutputMode,
    collapsedRootPaths: Set<String>,
    onCollapsedRootPathsChange: (Set<String>) -> Unit,
    onCanonicalEntryClick: (CanonicalCatalogEntry) -> Unit,
    onCanonicalParentClick: (String) -> Unit,
    parentSelectionEnabled: Boolean,
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
                if (parentSelectionEnabled) {
                    TextButton(onClick = { onCanonicalParentClick(node.path) }) {
                        Text(stringResource(R.string.record_action_select_parent))
                    }
                }
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
                        val displayLabel = entry.displayLabel(displayMode)
                        SuggestionChip(
                            onClick = { onCanonicalEntryClick(entry) },
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
                        onCanonicalEntryClick = onCanonicalEntryClick,
                        onCanonicalParentClick = onCanonicalParentClick,
                        parentSelectionEnabled = parentSelectionEnabled,
                        modifier = Modifier,
                        depth = depth + 1
                    )
                }
            }
        }
    }
}

internal fun String.displayCanonicalPathForUi(): String = this

internal fun CanonicalPathNode.displayTitle(depth: Int): String =
    if (depth == 0) {
        path.displayCanonicalPathForUi()
    } else {
        name.displayCanonicalPathForUi()
    }

internal fun CanonicalCatalogEntry.displayToken(displayMode: RecordSuggestionOutputMode): String {
    val canonical = canonicalPath.trim()
    if (displayMode != RecordSuggestionOutputMode.ALIAS) {
        return canonical
    }
    return aliases.firstOrNull { alias -> alias.isNotBlank() }?.trim().orEmpty().ifEmpty { canonical }
}

internal fun CanonicalCatalogEntry.displayLabel(displayMode: RecordSuggestionOutputMode): String {
    if (displayMode == RecordSuggestionOutputMode.ALIAS) {
        return displayToken(displayMode).displayCanonicalPathForUi()
    }
    return canonicalLeaf.trim().ifEmpty { canonicalPath.trim() }.displayCanonicalPathForUi()
}

internal data class RootDragMetrics(
    val topPx: Float,
    val heightPx: Int
) {
    val centerYPx: Float
        get() = topPx + (heightPx / 2f)
}

internal val CanonicalRootTagSanitizer = Regex("[^A-Za-z0-9]+")

internal fun calculateCanonicalRootDropIndex(
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

internal fun List<CanonicalPathNode>.orderCanonicalRoots(orderedRootPaths: List<String>): List<CanonicalPathNode> {
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
