package com.example.tracer

import android.graphics.Paint
import androidx.compose.foundation.Canvas
import androidx.compose.foundation.gestures.detectTapGestures
import androidx.compose.foundation.layout.BoxWithConstraints
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.runtime.Composable
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
import kotlin.math.max

private data class TreemapNodeRect(
    val index: Int,
    val slice: ReportCompositionSlice,
    val bounds: Rect
)

@Composable
internal fun ReportCompositionTreemapChart(
    slices: List<ReportCompositionSlice>,
    palettePreset: ReportPiePalettePreset,
    selectedIndex: Int,
    onItemSelected: (Int) -> Unit,
    modifier: Modifier = Modifier
) {
    val density = LocalDensity.current
    val colors = rememberCompositionSliceColors(
        slices = slices,
        palettePreset = palettePreset
    )
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
                val color = colors.getOrElse(node.index) { Color(0xFF4F46E5) }
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
                    roundRect = roundRect
                )
            }
        }
    }
}

private fun computeTreemapRects(
    slices: List<ReportCompositionSlice>,
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
    return sliceDiceTreemap(
        indexedSlices = indexedSlices,
        bounds = Rect(0f, 0f, widthPx, heightPx),
        totalDuration = totalDuration,
        vertical = widthPx >= heightPx
    )
}

private fun sliceDiceTreemap(
    indexedSlices: List<IndexedValue<ReportCompositionSlice>>,
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
    slice: ReportCompositionSlice,
    fillColor: Color,
    roundRect: RoundRect
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
            val canShowSubtitle = bounds.width >= 132f && bounds.height >= 82f
            if (canShowSubtitle) {
                val subtitle = ellipsizeForWidth(
                    text = formatDurationHoursMinutes(slice.durationSeconds),
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
