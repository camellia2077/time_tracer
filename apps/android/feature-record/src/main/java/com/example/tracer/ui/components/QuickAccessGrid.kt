package com.example.tracer

import androidx.compose.animation.core.Spring
import androidx.compose.animation.core.animateFloatAsState
import androidx.compose.animation.core.spring
import androidx.compose.foundation.gestures.detectDragGesturesAfterLongPress
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.BoxWithConstraints
import androidx.compose.foundation.layout.fillMaxHeight
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.offset
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.layout.widthIn
import androidx.compose.material3.InputChip
import androidx.compose.material3.InputChipDefaults
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.SuggestionChip
import androidx.compose.material3.SuggestionChipDefaults
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.key
import androidx.compose.runtime.mutableStateMapOf
import androidx.compose.runtime.mutableIntStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.graphics.graphicsLayer
import androidx.compose.ui.hapticfeedback.HapticFeedbackType
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.layout.onGloballyPositioned
import androidx.compose.ui.platform.LocalDensity
import androidx.compose.ui.platform.LocalHapticFeedback
import androidx.compose.ui.platform.testTag
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.IntOffset
import androidx.compose.ui.unit.dp
import androidx.compose.ui.zIndex
import com.example.tracer.feature.record.R
import com.example.tracer.ui.components.SimpleFlowRow
import kotlin.math.roundToInt

@Composable
internal fun QuickAccessActivityGrid(
    modifier: Modifier = Modifier,
    quickActivities: List<String>,
    recordContent: String,
    isDeleteMode: Boolean,
    onRecordContentChange: (String) -> Unit,
    onQuickActivitiesUpdate: (List<String>) -> Boolean
) {
    var displayedActivities by remember { mutableStateOf(quickActivities) }
    var draggedActivity by remember { mutableStateOf<String?>(null) }
    var draggedIndex by remember { mutableIntStateOf(-1) }
    var dragOffsetPx by remember { mutableStateOf(Offset.Zero) }
    val measuredWidthsPx = remember { mutableStateMapOf<String, Int>() }

    LaunchedEffect(quickActivities, isDeleteMode) {
        if (draggedActivity == null) {
            displayedActivities = quickActivities
        }
        measuredWidthsPx.keys
            .filter { it !in quickActivities }
            .toList()
            .forEach(measuredWidthsPx::remove)
    }

    val hapticFeedback = LocalHapticFeedback.current
    val density = LocalDensity.current

    BoxWithConstraints(modifier = modifier.fillMaxWidth()) {
        val maxChipWidth = maxWidth.coerceAtMost(QuickAccessGridMaxChipWidth)
        val containerWidthPx = with(density) { maxWidth.roundToPx() }
        val maxChipWidthPx = with(density) { maxChipWidth.roundToPx() }.coerceAtLeast(1)
        val cellHeightPx = with(density) { QuickAccessGridCellHeight.roundToPx() }
        val horizontalGapPx = with(density) { QuickAccessGridHorizontalGap.roundToPx() }
        val verticalGapPx = with(density) { QuickAccessGridVerticalGap.roundToPx() }
        val draggedShadowPx = with(density) { 12.dp.toPx() }
        val widthSnapshot = displayedActivities.associateWith { activity ->
            measuredWidthsPx[activity]
        }
        val measuredWidthMap = widthSnapshot.mapNotNull { (activity, widthPx) ->
            widthPx?.let { activity to it }
        }.toMap()
        val allWidthsMeasured = displayedActivities.all { activity ->
            widthSnapshot[activity] != null
        }

        fun finishDrag() {
            val reorderedActivities = displayedActivities
            val didChange = reorderedActivities != quickActivities
            draggedActivity = null
            draggedIndex = -1
            dragOffsetPx = Offset.Zero
            if (!didChange) {
                displayedActivities = quickActivities
                return
            }
            if (!onQuickActivitiesUpdate(reorderedActivities)) {
                displayedActivities = quickActivities
            }
        }

        if (isDeleteMode) {
            SimpleFlowRow(
                modifier = Modifier
                    .fillMaxWidth()
                    .testTag(quickAccessGridTestTag()),
                horizontalGap = QuickAccessGridHorizontalGap,
                verticalGap = QuickAccessGridVerticalGap
            ) {
                displayedActivities.forEach { activity ->
                    key(activity) {
                        Box(
                            modifier = Modifier
                                .widthIn(max = maxChipWidth)
                                .height(QuickAccessGridCellHeight)
                                .testTag(quickAccessItemTestTag(activity))
                        ) {
                            DeleteQuickAccessChip(
                                activity = activity,
                                recordContent = recordContent,
                                quickActivities = quickActivities,
                                onQuickActivitiesUpdate = onQuickActivitiesUpdate,
                                modifier = Modifier.fillMaxHeight()
                            )
                        }
                    }
                }
            }
        } else if (!allWidthsMeasured) {
            SimpleFlowRow(
                modifier = Modifier
                    .fillMaxWidth()
                    .testTag(quickAccessGridTestTag()),
                horizontalGap = QuickAccessGridHorizontalGap,
                verticalGap = QuickAccessGridVerticalGap
            ) {
                displayedActivities.forEach { activity ->
                    key(activity) {
                        Box(
                            modifier = Modifier
                                .widthIn(max = maxChipWidth)
                                .height(QuickAccessGridCellHeight)
                                .onGloballyPositioned { coordinates ->
                                    measuredWidthsPx[activity] = coordinates.size.width
                                }
                                .testTag(quickAccessItemTestTag(activity))
                        ) {
                            QuickAccessSuggestionChip(
                                activity = activity,
                                recordContent = recordContent,
                                onRecordContentChange = onRecordContentChange,
                                modifier = Modifier.fillMaxHeight()
                            )
                        }
                    }
                }
            }
        } else {
            val flowLayout = calculateQuickAccessFlowLayout(
                activities = displayedActivities,
                measuredWidthsPx = measuredWidthMap,
                containerWidthPx = containerWidthPx,
                maxItemWidthPx = maxChipWidthPx,
                itemHeightPx = cellHeightPx,
                horizontalGapPx = horizontalGapPx,
                verticalGapPx = verticalGapPx
            )
            val slotsByActivity = flowLayout.slots.associateBy(QuickAccessSlot::activity)

            Box(
                modifier = Modifier
                    .fillMaxWidth()
                    .height(with(density) { flowLayout.totalHeightPx.toDp() })
                    .testTag(quickAccessGridTestTag())
            ) {
                displayedActivities.forEach { activity ->
                    key(activity) {
                        val slot = slotsByActivity.getValue(activity)
                        val isDragging = draggedActivity == activity
                        val animatedX by animateFloatAsState(
                            targetValue = slot.topLeftPx.x,
                            animationSpec = spring(
                                stiffness = Spring.StiffnessMediumLow,
                                dampingRatio = Spring.DampingRatioNoBouncy
                            ),
                            label = "quick_access_x_$activity"
                        )
                        val animatedY by animateFloatAsState(
                            targetValue = slot.topLeftPx.y,
                            animationSpec = spring(
                                stiffness = Spring.StiffnessMediumLow,
                                dampingRatio = Spring.DampingRatioNoBouncy
                            ),
                            label = "quick_access_y_$activity"
                        )
                        val visualOffset = if (isDragging) {
                            slot.topLeftPx + dragOffsetPx
                        } else {
                            Offset(animatedX, animatedY)
                        }
                        val dragModifier = Modifier.pointerInput(
                            activity,
                            maxChipWidthPx,
                            containerWidthPx
                        ) {
                            detectDragGesturesAfterLongPress(
                                onDragStart = {
                                    hapticFeedback.performHapticFeedback(HapticFeedbackType.LongPress)
                                    draggedActivity = activity
                                    draggedIndex = displayedActivities.indexOf(activity)
                                    dragOffsetPx = Offset.Zero
                                },
                                onDragCancel = ::finishDrag,
                                onDragEnd = ::finishDrag,
                                onDrag = { change, dragAmount ->
                                    if (draggedActivity != activity || draggedIndex !in displayedActivities.indices) {
                                        return@detectDragGesturesAfterLongPress
                                    }
                                    val currentLayout = calculateQuickAccessFlowLayout(
                                        activities = displayedActivities,
                                        measuredWidthsPx = measuredWidthMap,
                                        containerWidthPx = containerWidthPx,
                                        maxItemWidthPx = maxChipWidthPx,
                                        itemHeightPx = cellHeightPx,
                                        horizontalGapPx = horizontalGapPx,
                                        verticalGapPx = verticalGapPx
                                    )
                                    val currentSlot = currentLayout.slots.firstOrNull { it.activity == activity }
                                        ?: return@detectDragGesturesAfterLongPress
                                    change.consume()
                                    dragOffsetPx += Offset(dragAmount.x, dragAmount.y)
                                    val targetIndex = calculateQuickAccessDropIndex(
                                        pointerCenter = currentSlot.centerPx + dragOffsetPx,
                                        slots = currentLayout.slots
                                    )
                                    if (targetIndex == draggedIndex || targetIndex < 0) {
                                        return@detectDragGesturesAfterLongPress
                                    }
                                    val reorderedActivities = displayedActivities.moveItem(
                                        fromIndex = draggedIndex,
                                        toIndex = targetIndex
                                    )
                                    val reorderedLayout = calculateQuickAccessFlowLayout(
                                        activities = reorderedActivities,
                                        measuredWidthsPx = measuredWidthMap,
                                        containerWidthPx = containerWidthPx,
                                        maxItemWidthPx = maxChipWidthPx,
                                        itemHeightPx = cellHeightPx,
                                        horizontalGapPx = horizontalGapPx,
                                        verticalGapPx = verticalGapPx
                                    )
                                    val newSlot = reorderedLayout.slots.firstOrNull { it.activity == activity }
                                        ?: return@detectDragGesturesAfterLongPress
                                    displayedActivities = reorderedActivities
                                    dragOffsetPx += currentSlot.topLeftPx - newSlot.topLeftPx
                                    draggedIndex = targetIndex
                                }
                            )
                        }

                        Box(
                            modifier = Modifier
                                .width(with(density) { slot.widthPx.toDp() })
                                .height(QuickAccessGridCellHeight)
                                .offset {
                                    IntOffset(
                                        x = visualOffset.x.roundToInt(),
                                        y = visualOffset.y.roundToInt()
                                    )
                                }
                                .zIndex(if (isDragging) 1f else 0f)
                                .graphicsLayer {
                                    if (isDragging) {
                                        scaleX = 1.03f
                                        scaleY = 1.03f
                                        shadowElevation = draggedShadowPx
                                    }
                                }
                                .testTag(quickAccessItemTestTag(activity))
                                .then(dragModifier)
                        ) {
                            QuickAccessSuggestionChip(
                                activity = activity,
                                recordContent = recordContent,
                                onRecordContentChange = onRecordContentChange,
                                modifier = Modifier.fillMaxSize()
                            )
                        }
                    }
                }
            }
        }
    }
}

@Composable
private fun QuickAccessSuggestionChip(
    activity: String,
    recordContent: String,
    onRecordContentChange: (String) -> Unit,
    modifier: Modifier
) {
    val isSelected = recordContent.trim() == activity
    SuggestionChip(
        onClick = { onRecordContentChange(activity) },
        modifier = modifier,
        label = {
            Text(
                text = activity,
                style = MaterialTheme.typography.bodyMedium,
                modifier = Modifier.padding(horizontal = 4.dp),
                maxLines = 1,
                overflow = TextOverflow.Ellipsis
            )
        },
        colors = SuggestionChipDefaults.suggestionChipColors(
            containerColor = if (isSelected) {
                MaterialTheme.colorScheme.primaryContainer
            } else {
                MaterialTheme.colorScheme.surfaceContainerHigh
            },
            labelColor = if (isSelected) {
                MaterialTheme.colorScheme.onPrimaryContainer
            } else {
                MaterialTheme.colorScheme.onSurface
            }
        ),
        border = SuggestionChipDefaults.suggestionChipBorder(
            enabled = true,
            borderColor = if (isSelected) {
                MaterialTheme.colorScheme.primary
            } else {
                MaterialTheme.colorScheme.outline
            }
        )
    )
}

@Composable
private fun DeleteQuickAccessChip(
    activity: String,
    recordContent: String,
    quickActivities: List<String>,
    onQuickActivitiesUpdate: (List<String>) -> Boolean,
    modifier: Modifier
) {
    InputChip(
        selected = recordContent.trim() == activity,
        onClick = {
            onQuickActivitiesUpdate(
                quickActivities.filter { it != activity }
            )
        },
        modifier = modifier,
        label = {
            Text(
                text = stringResource(
                    R.string.record_chip_remove_activity,
                    activity
                ),
                maxLines = 1,
                overflow = TextOverflow.Ellipsis
            )
        },
        colors = InputChipDefaults.inputChipColors(
            containerColor = MaterialTheme.colorScheme.errorContainer,
            labelColor = MaterialTheme.colorScheme.onErrorContainer
        )
    )
}
