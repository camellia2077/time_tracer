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
fun RecordCanonicalCatalogScreen(
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
    onCanonicalEntryClick: (CanonicalCatalogEntry) -> Unit,
    onCanonicalParentClick: (String) -> Unit = {}
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
                                } else if (target == CanonicalBrowserTarget.INSIGHTS_STATUS_PARENT) {
                                    R.string.record_canonical_catalog_parent_title
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
                        onCanonicalEntryClick = onCanonicalEntryClick,
                        onCanonicalParentClick = onCanonicalParentClick,
                        modifier = Modifier
                            .fillMaxWidth()
                            .padding(start = 24.dp, end = 24.dp, top = 16.dp, bottom = 24.dp)
                    )
                }
            }
        }
    }
}

