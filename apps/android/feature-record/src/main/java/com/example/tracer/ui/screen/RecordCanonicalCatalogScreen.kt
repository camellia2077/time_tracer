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
import androidx.compose.material3.PrimaryTabRow
import androidx.compose.material3.Surface
import androidx.compose.material3.SuggestionChip
import androidx.compose.material3.Tab
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
import androidx.compose.ui.zIndex
import com.example.tracer.feature.record.R
import java.time.Clock
import kotlin.math.abs

enum class CanonicalCatalogSource {
    TREE,
    FREQUENT,
    CATEGORIES
}

@Composable
fun RecordCanonicalCatalogScreen(
    isLoading: Boolean,
    roots: List<CanonicalPathNode>,
    statusText: String,
    displayMode: RecordFrequentOutputMode,
    source: CanonicalCatalogSource,
    onSourceChange: (CanonicalCatalogSource) -> Unit,
    isFrequentActivitiesLoading: Boolean,
    frequentActivities: List<RecordFrequentActivity>,
    frequentLookbackDays: Int,
    frequentTopN: Int,
    onFrequentActivitiesRequested: () -> Unit,
    onFrequentLookbackDaysChange: (String) -> Unit,
    onFrequentTopNChange: (String) -> Unit,
    onFrequentActivityClick: (String) -> Boolean,
    onTreeRequested: () -> Unit,
    searchQuery: String,
    onSearchQueryChange: (String) -> Unit,
    categoriesContent: @Composable () -> Unit,
    collapsedRootPaths: Set<String>,
    orderedRootPaths: List<String>,
    onDismissRequest: () -> Unit,
    onDisplayModeChange: (RecordFrequentOutputMode) -> Unit,
    onCollapsedRootPathsChange: (Set<String>) -> Unit,
    onOrderedRootPathsChange: (List<String>) -> Unit,
    onCanonicalEntryClick: (CanonicalCatalogEntry) -> Unit,
    onCanonicalParentClick: (String) -> Unit = {}
) {
    var isTreeFullscreen by remember { mutableStateOf(false) }
    var frequentLookbackDaysInput by remember(frequentLookbackDays) {
        mutableStateOf(frequentLookbackDays.toString())
    }
    var frequentTopNInput by remember(frequentTopN) {
        mutableStateOf(frequentTopN.toString())
    }
    FullscreenPage(onDismissRequest = onDismissRequest) {
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
                            text = stringResource(R.string.record_canonical_catalog_title),
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
                    PrimaryTabRow(selectedTabIndex = source.ordinal) {
                            Tab(
                                selected = source == CanonicalCatalogSource.TREE,
                                onClick = {
                                    onSourceChange(CanonicalCatalogSource.TREE)
                                    onTreeRequested()
                                },
                                text = { Text(stringResource(R.string.record_canonical_catalog_source_tree)) }
                            )
                            Tab(
                                selected = source == CanonicalCatalogSource.FREQUENT,
                                onClick = {
                                    onSourceChange(CanonicalCatalogSource.FREQUENT)
                                    onFrequentActivitiesRequested()
                                },
                                text = { Text(stringResource(R.string.record_canonical_catalog_source_frequent)) }
                            )
                            Tab(
                                selected = source == CanonicalCatalogSource.CATEGORIES,
                                onClick = {
                                    onSourceChange(CanonicalCatalogSource.CATEGORIES)
                                },
                                text = { Text(stringResource(R.string.record_canonical_catalog_source_categories)) }
                            )
                    }
                    if (source == CanonicalCatalogSource.TREE) Row(
                        modifier = Modifier.fillMaxWidth(),
                        horizontalArrangement = Arrangement.spacedBy(8.dp),
                        verticalAlignment = Alignment.CenterVertically
                    ) {
                        OutlinedTextField(
                            value = searchQuery,
                            onValueChange = onSearchQueryChange,
                            label = { Text(stringResource(R.string.record_canonical_catalog_search_label)) },
                            singleLine = true,
                            shape = MaterialTheme.shapes.small,
                            modifier = Modifier
                                .weight(1f)
                                .testTag("record_canonical_catalog_search")
                        )
                        if (displayMode == RecordFrequentOutputMode.CANONICAL) {
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
                                    onDisplayModeChange(RecordFrequentOutputMode.CANONICAL)
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
                        if (displayMode == RecordFrequentOutputMode.ALIAS) {
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
                                    onDisplayModeChange(RecordFrequentOutputMode.ALIAS)
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
                    if (source == CanonicalCatalogSource.FREQUENT) {
                        Row(
                            modifier = Modifier.fillMaxWidth(),
                            horizontalArrangement = Arrangement.spacedBy(8.dp)
                        ) {
                            if (displayMode == RecordFrequentOutputMode.CANONICAL) {
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
                                        onDisplayModeChange(RecordFrequentOutputMode.CANONICAL)
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
                            if (displayMode == RecordFrequentOutputMode.ALIAS) {
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
                                        onDisplayModeChange(RecordFrequentOutputMode.ALIAS)
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
                            text = stringResource(R.string.record_canonical_catalog_frequent_description),
                            style = MaterialTheme.typography.bodyMedium,
                            color = MaterialTheme.colorScheme.onSurfaceVariant
                        )
                        Row(
                            modifier = Modifier.fillMaxWidth(),
                            horizontalArrangement = Arrangement.spacedBy(8.dp)
                        ) {
                                OutlinedTextField(
                                    value = frequentLookbackDaysInput,
                                    onValueChange = { rawValue ->
                                        val digitsOnly = rawValue.filter(Char::isDigit)
                                        frequentLookbackDaysInput = digitsOnly
                                        if (digitsOnly.toIntOrNull() != null) {
                                            onFrequentLookbackDaysChange(digitsOnly)
                                        }
                                    },
                                    label = { Text(stringResource(R.string.record_label_days)) },
                                    modifier = Modifier.weight(1f),
                                    keyboardOptions = KeyboardOptions(
                                        keyboardType = KeyboardType.Number
                                    )
                                )
                                OutlinedTextField(
                                    value = frequentTopNInput,
                                    onValueChange = { rawValue ->
                                        val digitsOnly = rawValue.filter(Char::isDigit)
                                        frequentTopNInput = digitsOnly
                                        if (digitsOnly.toIntOrNull() != null) {
                                            onFrequentTopNChange(digitsOnly)
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
                    if (source == CanonicalCatalogSource.TREE) CanonicalActivityTree(
                        isLoading = isLoading,
                        roots = roots,
                        statusText = statusText,
                        displayMode = displayMode,
                        target = CanonicalBrowserTarget.RECORD_INPUT,
                        collapsedRootPaths = collapsedRootPaths,
                        orderedRootPaths = orderedRootPaths,
                        onCollapsedRootPathsChange = onCollapsedRootPathsChange,
                        onOrderedRootPathsChange = onOrderedRootPathsChange,
                        onCanonicalEntryClick = onCanonicalEntryClick,
                        onCanonicalParentClick = onCanonicalParentClick,
                        onOpenFullscreen = { isTreeFullscreen = true },
                        modifier = Modifier
                            .fillMaxWidth()
                            .padding(start = 24.dp, end = 24.dp, top = 16.dp, bottom = 24.dp)
                    ) else if (source == CanonicalCatalogSource.FREQUENT) FrequentActivitiesList(
                        isLoading = isFrequentActivitiesLoading,
                        activities = frequentActivities,
                        displayMode = displayMode,
                        emptyText = stringResource(R.string.record_hint_no_frequent_activities),
                        onActivityClick = { onFrequentActivityClick(it) },
                        modifier = Modifier
                            .fillMaxWidth()
                            .padding(start = 24.dp, end = 24.dp, top = 16.dp, bottom = 24.dp)
                    ) else categoriesContent()
                }
        }
    }
    if (isTreeFullscreen) {
        FullscreenPage(onDismissRequest = { isTreeFullscreen = false }) {
            Box(
                modifier = Modifier
                    .fillMaxSize()
                    .verticalScroll(rememberScrollState())
            ) {
                CanonicalActivityTree(
                    isLoading = isLoading,
                    roots = roots,
                    statusText = statusText,
                    displayMode = displayMode,
                    target = CanonicalBrowserTarget.RECORD_INPUT,
                    collapsedRootPaths = collapsedRootPaths,
                    orderedRootPaths = orderedRootPaths,
                    onCollapsedRootPathsChange = onCollapsedRootPathsChange,
                    onOrderedRootPathsChange = onOrderedRootPathsChange,
                    onCanonicalEntryClick = onCanonicalEntryClick,
                    onCanonicalParentClick = onCanonicalParentClick,
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(24.dp)
                )
            }
        }
    }
}
