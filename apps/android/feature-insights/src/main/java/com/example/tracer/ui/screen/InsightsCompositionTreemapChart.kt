package com.example.tracer

import android.graphics.Paint
import androidx.compose.foundation.Canvas
import androidx.compose.foundation.gestures.detectTapGestures
import androidx.compose.foundation.layout.BoxWithConstraints
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.remember
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.geometry.CornerRadius
import androidx.compose.ui.geometry.Rect
import androidx.compose.ui.geometry.RoundRect
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.Path
import androidx.compose.ui.graphics.drawscope.Stroke
import androidx.compose.ui.graphics.drawscope.clipPath
import androidx.compose.ui.graphics.luminance
import androidx.compose.ui.graphics.nativeCanvas
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.platform.LocalDensity
import androidx.compose.ui.unit.dp
import kotlin.math.abs
import kotlin.math.max
import kotlin.math.min

internal data class TreemapNodeRect(
    val index: Int,
    val slice: InsightsCompositionSlice,
    val bounds: Rect
)

@Composable
internal fun InsightsCompositionTreemapChart(
    slices: List<InsightsCompositionSlice>,
    palettePreset: InsightsPiePalettePreset,
    selectedIndex: Int,
    onItemSelected: (Int) -> Unit,
    onSliceColorsResolved: (List<Color>) -> Unit = {},
    valueLabel: (Long) -> String = ::formatDurationHoursMinutes,
    modifier: Modifier = Modifier
) {
    val density = LocalDensity.current
    BoxWithConstraints(modifier = modifier) {
        val widthPx = with(density) { maxWidth.toPx() }
        val heightPx = with(density) { maxHeight.toPx() }
        val layout = remember(slices, widthPx, heightPx) {
            computeTreemapRects(
                slices = slices,
                widthPx = widthPx,
                heightPx = heightPx
            )
        }
        val colors = remember(layout, palettePreset) {
            resolveTreemapColors(
                layout = layout,
                palettePreset = palettePreset
            )
        }
        LaunchedEffect(colors, slices) {
            onSliceColorsResolved(
                slices.indices.map { index -> colors[index] ?: Color(0xFF4F46E5) }
            )
        }
        Canvas(
            modifier = Modifier
                .fillMaxSize()
                .pointerInput(layout) {
                    detectTapGestures { tapOffset ->
                        layout.indexOfFirst { it.bounds.contains(tapOffset) }
                            .takeIf { it >= 0 }
                            ?.let(onItemSelected)
                    }
                }
        ) {
            val gapPx = 4.dp.toPx()
            val cornerRadiusPx = 14.dp.toPx()
            val strokeWidthPx = 2.dp.toPx()
            layout.forEach { node ->
                val color = colors[node.index] ?: Color(0xFF4F46E5)
                val insetBounds = Rect(
                    left = node.bounds.left + gapPx,
                    top = node.bounds.top + gapPx,
                    right = node.bounds.right - gapPx,
                    bottom = node.bounds.bottom - gapPx
                )
                if (insetBounds.width <= 0f || insetBounds.height <= 0f) {
                    return@forEach
                }
                val roundRect = RoundRect(
                    rect = insetBounds,
                    cornerRadius = CornerRadius(cornerRadiusPx, cornerRadiusPx)
                )
                drawRoundRect(
                    color = color,
                    topLeft = insetBounds.topLeft,
                    size = insetBounds.size,
                    cornerRadius = androidx.compose.ui.geometry.CornerRadius(
                        x = cornerRadiusPx,
                        y = cornerRadiusPx
                    )
                )
                if (selectedIndex == node.index) {
                    drawRoundRect(
                        color = Color.White.copy(alpha = 0.92f),
                        topLeft = insetBounds.topLeft,
                        size = insetBounds.size,
                        cornerRadius = androidx.compose.ui.geometry.CornerRadius(
                            x = cornerRadiusPx,
                            y = cornerRadiusPx
                        ),
                        style = Stroke(width = strokeWidthPx)
                    )
                }
                drawTreemapLabel(
                    bounds = insetBounds,
                    slice = node.slice,
                    fillColor = color,
                    roundRect = roundRect,
                    showDuration = shouldShowTreemapDuration(node.index),
                    valueLabel = valueLabel
                )
            }
        }
    }
}

internal fun computeTreemapRects(
    slices: List<InsightsCompositionSlice>,
    widthPx: Float,
    heightPx: Float
): List<TreemapNodeRect> {
    if (slices.isEmpty() || widthPx <= 0f || heightPx <= 0f) {
        return emptyList()
    }
    val indexedSlices = slices.withIndex().filter { it.value.durationSeconds > 0L }
    val totalDuration = indexedSlices.sumOf { it.value.durationSeconds }.toFloat()
    if (totalDuration <= 0f) {
        return emptyList()
    }
    return rankedTreemap(
        indexedSlices = indexedSlices,
        bounds = Rect(0f, 0f, widthPx, heightPx),
        totalDuration = totalDuration
    )
}

internal fun resolveTreemapColors(
    layout: List<TreemapNodeRect>,
    palettePreset: InsightsPiePalettePreset
): Map<Int, Color> {
    val palette = resolveInsightsBreakdownPaletteColors(palettePreset)
    val colorsByIndex = mutableMapOf<Int, Color>()
    layout
        .sortedWith(
            compareByDescending<TreemapNodeRect> { it.slice.durationSeconds }
                .thenBy { it.index }
        )
        .forEachIndexed { rank, node ->
            val neighboringColors = layout.asSequence()
                .filter { candidate ->
                    candidate.index in colorsByIndex &&
                        areTreemapNodesAdjacent(node, candidate)
                }
                .mapNotNull { candidate -> colorsByIndex[candidate.index] }
                .toSet()
            val baseColor = palette[rank.mod(palette.size)]
            colorsByIndex[node.index] = if (baseColor !in neighboringColors) {
                baseColor
            } else {
                palette.firstOrNull { color -> color !in neighboringColors } ?: baseColor
            }
        }
    return colorsByIndex
}

internal fun areTreemapNodesAdjacent(
    first: TreemapNodeRect,
    second: TreemapNodeRect
): Boolean {
    val firstBounds = first.bounds
    val secondBounds = second.bounds
    val sharesVerticalEdge =
        abs(firstBounds.right - secondBounds.left) <= TREEMAP_ADJACENCY_EPSILON_PX ||
            abs(secondBounds.right - firstBounds.left) <= TREEMAP_ADJACENCY_EPSILON_PX
    val sharesHorizontalEdge =
        abs(firstBounds.bottom - secondBounds.top) <= TREEMAP_ADJACENCY_EPSILON_PX ||
            abs(secondBounds.bottom - firstBounds.top) <= TREEMAP_ADJACENCY_EPSILON_PX
    val verticalOverlap = min(firstBounds.bottom, secondBounds.bottom) -
        max(firstBounds.top, secondBounds.top)
    val horizontalOverlap = min(firstBounds.right, secondBounds.right) -
        max(firstBounds.left, secondBounds.left)
    return (sharesVerticalEdge && verticalOverlap > TREEMAP_ADJACENCY_EPSILON_PX) ||
        (sharesHorizontalEdge && horizontalOverlap > TREEMAP_ADJACENCY_EPSILON_PX)
}

private const val TREEMAP_ADJACENCY_EPSILON_PX = 0.5f
private const val TREEMAP_DURATION_LABEL_COUNT = 4

internal fun shouldShowTreemapDuration(rank: Int): Boolean =
    rank in 0 until TREEMAP_DURATION_LABEL_COUNT

private fun rankedTreemap(
    indexedSlices: List<IndexedValue<InsightsCompositionSlice>>,
    bounds: Rect,
    totalDuration: Float
): List<TreemapNodeRect> {
    if (indexedSlices.size < 3) {
        return sliceDiceTreemap(
            indexedSlices = indexedSlices,
            bounds = bounds,
            totalDuration = totalDuration,
            vertical = bounds.width >= bounds.height
        )
    }

    val first = indexedSlices[0]
    val second = indexedSlices[1]
    val third = indexedSlices[2]
    val tail = indexedSlices.drop(3)
    val firstRatio = (first.value.durationSeconds.toFloat() / totalDuration).coerceIn(0f, 1f)
    val remainingTotal = (totalDuration - first.value.durationSeconds.toFloat()).coerceAtLeast(0f)
    if (remainingTotal <= 0f) {
        return sliceDiceTreemap(
            indexedSlices = indexedSlices,
            bounds = bounds,
            totalDuration = totalDuration,
            vertical = bounds.width >= bounds.height
        )
    }

    return if (bounds.width >= bounds.height) {
        layoutLandscapeRankedTreemap(first, second, third, tail, bounds, firstRatio, remainingTotal)
    } else {
        layoutPortraitRankedTreemap(first, second, third, tail, bounds, firstRatio, remainingTotal)
    }
}

private fun layoutLandscapeRankedTreemap(
    first: IndexedValue<InsightsCompositionSlice>,
    second: IndexedValue<InsightsCompositionSlice>,
    third: IndexedValue<InsightsCompositionSlice>,
    tail: List<IndexedValue<InsightsCompositionSlice>>,
    bounds: Rect,
    firstRatio: Float,
    remainingTotal: Float
): List<TreemapNodeRect> {
    val firstRight = bounds.left + bounds.width * firstRatio
    val remainder = Rect(firstRight, bounds.top, bounds.right, bounds.bottom)
    val secondBottom = remainder.top + remainder.height *
        (second.value.durationSeconds.toFloat() / remainingTotal).coerceIn(0f, 1f)
    val thirdBottom = secondBottom + remainder.height *
        (third.value.durationSeconds.toFloat() / remainingTotal).coerceIn(0f, 1f)
    val secondBounds = Rect(remainder.left, remainder.top, remainder.right, secondBottom)
    val thirdBounds = Rect(remainder.left, secondBottom, remainder.right, thirdBottom)
    val tailBounds = Rect(remainder.left, thirdBottom, remainder.right, remainder.bottom)
    return listOf(
        TreemapNodeRect(first.index, first.value, Rect(bounds.left, bounds.top, firstRight, bounds.bottom)),
        TreemapNodeRect(second.index, second.value, secondBounds),
        TreemapNodeRect(third.index, third.value, thirdBounds)
    ) + layoutTreemapTail(tail, tailBounds)
}

private fun layoutPortraitRankedTreemap(
    first: IndexedValue<InsightsCompositionSlice>,
    second: IndexedValue<InsightsCompositionSlice>,
    third: IndexedValue<InsightsCompositionSlice>,
    tail: List<IndexedValue<InsightsCompositionSlice>>,
    bounds: Rect,
    firstRatio: Float,
    remainingTotal: Float
): List<TreemapNodeRect> {
    val firstBottom = bounds.top + bounds.height * firstRatio
    val remainder = Rect(bounds.left, firstBottom, bounds.right, bounds.bottom)
    val secondRight = remainder.left + remainder.width *
        (second.value.durationSeconds.toFloat() / remainingTotal).coerceIn(0f, 1f)
    val thirdRight = secondRight + remainder.width *
        (third.value.durationSeconds.toFloat() / remainingTotal).coerceIn(0f, 1f)
    val secondBounds = Rect(remainder.left, remainder.top, secondRight, remainder.bottom)
    val thirdBounds = Rect(secondRight, remainder.top, thirdRight, remainder.bottom)
    val tailBounds = Rect(thirdRight, remainder.top, remainder.right, remainder.bottom)
    return listOf(
        TreemapNodeRect(first.index, first.value, Rect(bounds.left, bounds.top, bounds.right, firstBottom)),
        TreemapNodeRect(second.index, second.value, secondBounds),
        TreemapNodeRect(third.index, third.value, thirdBounds)
    ) + layoutTreemapTail(tail, tailBounds)
}

private fun layoutTreemapTail(
    tail: List<IndexedValue<InsightsCompositionSlice>>,
    bounds: Rect
): List<TreemapNodeRect> {
    if (tail.isEmpty()) {
        return emptyList()
    }
    return sliceDiceTreemap(
        indexedSlices = tail,
        bounds = bounds,
        totalDuration = tail.sumOf { it.value.durationSeconds }.toFloat(),
        vertical = bounds.width >= bounds.height
    )
}

private fun sliceDiceTreemap(
    indexedSlices: List<IndexedValue<InsightsCompositionSlice>>,
    bounds: Rect,
    totalDuration: Float,
    vertical: Boolean
): List<TreemapNodeRect> {
    if (indexedSlices.isEmpty() || bounds.width <= 0f || bounds.height <= 0f) {
        return emptyList()
    }
    if (indexedSlices.size == 1) {
        val only = indexedSlices.first()
        return listOf(
            TreemapNodeRect(
                index = only.index,
                slice = only.value,
                bounds = bounds
            )
        )
    }

    val head = indexedSlices.first()
    val tail = indexedSlices.drop(1)
    val ratio = head.value.durationSeconds.toFloat() / totalDuration
    return if (vertical) {
        val splitWidth = bounds.width * ratio.coerceIn(0f, 1f)
        val headBounds = Rect(bounds.left, bounds.top, bounds.left + splitWidth, bounds.bottom)
        val tailBounds = Rect(bounds.left + splitWidth, bounds.top, bounds.right, bounds.bottom)
        listOf(
            TreemapNodeRect(
                index = head.index,
                slice = head.value,
                bounds = headBounds
            )
        ) + sliceDiceTreemap(
            indexedSlices = tail,
            bounds = tailBounds,
            totalDuration = max(totalDuration - head.value.durationSeconds.toFloat(), 0f),
            vertical = !vertical
        )
    } else {
        val splitHeight = bounds.height * ratio.coerceIn(0f, 1f)
        val headBounds = Rect(bounds.left, bounds.top, bounds.right, bounds.top + splitHeight)
        val tailBounds = Rect(bounds.left, bounds.top + splitHeight, bounds.right, bounds.bottom)
        listOf(
            TreemapNodeRect(
                index = head.index,
                slice = head.value,
                bounds = headBounds
            )
        ) + sliceDiceTreemap(
            indexedSlices = tail,
            bounds = tailBounds,
            totalDuration = max(totalDuration - head.value.durationSeconds.toFloat(), 0f),
            vertical = !vertical
        )
    }
}

private fun androidx.compose.ui.graphics.drawscope.DrawScope.drawTreemapLabel(
    bounds: Rect,
    slice: InsightsCompositionSlice,
    fillColor: Color,
    roundRect: RoundRect,
    showDuration: Boolean,
    valueLabel: (Long) -> String
) {
    if (bounds.width < 88f || bounds.height < 44f) {
        return
    }
    val textColor = if (fillColor.luminance() > 0.55f) {
        android.graphics.Color.BLACK
    } else {
        android.graphics.Color.WHITE
    }
    val labelPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        color = textColor
        textSize = if (bounds.height >= 92f) 30f else 24f
        isFakeBoldText = true
    }
    val detailPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        color = textColor
        textSize = if (bounds.height >= 92f) 24f else 20f
    }
    val clipPath = Path().apply { addRoundRect(roundRect) }
    clipPath(clipPath) {
        drawContext.canvas.nativeCanvas.apply {
            val paddingX = 12.dp.toPx()
            val availableTextWidth = (bounds.width - paddingX * 2).coerceAtLeast(0f)
            if (availableTextWidth <= 0f) {
                return@apply
            }
            val titleY = bounds.top + 18.dp.toPx() + labelPaint.textSize * 0.7f
            // Treemap tiles can become tall and narrow, so label density must degrade by
            // available space: large tiles get name+summary, medium tiles get name only,
            // and smaller tiles leave full details to the shared selected-detail section.
            val title = ellipsizeForWidth(
                text = slice.root,
                paint = labelPaint,
                maxWidthPx = availableTextWidth
            )
            drawText(title, bounds.left + paddingX, titleY, labelPaint)
            if (showDuration) {
                val subtitle = ellipsizeForWidth(
                    text = valueLabel(slice.durationSeconds),
                    paint = detailPaint,
                    maxWidthPx = availableTextWidth
                )
                drawText(
                    subtitle,
                    bounds.left + paddingX,
                    titleY + 12.dp.toPx() + detailPaint.textSize,
                    detailPaint
                )
            }
        }
    }
}

private fun ellipsizeForWidth(
    text: String,
    paint: Paint,
    maxWidthPx: Float
): String {
    if (text.isEmpty() || maxWidthPx <= 0f) {
        return ""
    }
    if (paint.measureText(text) <= maxWidthPx) {
        return text
    }
    val ellipsis = "..."
    val ellipsisWidth = paint.measureText(ellipsis)
    if (ellipsisWidth >= maxWidthPx) {
        return ""
    }
    val measuredChars = paint.breakText(
        text,
        true,
        maxWidthPx - ellipsisWidth,
        null
    ).coerceAtLeast(0)
    return if (measuredChars <= 0) {
        ""
    } else {
        text.take(measuredChars).trimEnd() + ellipsis
    }
}
