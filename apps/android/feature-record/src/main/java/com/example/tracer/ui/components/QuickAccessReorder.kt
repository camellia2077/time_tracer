package com.example.tracer

import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.dp

internal val QuickAccessGridHorizontalGap: Dp = 8.dp
internal val QuickAccessGridVerticalGap: Dp = 8.dp
internal val QuickAccessGridCellHeight: Dp = 40.dp
internal val QuickAccessGridMaxChipWidth: Dp = 240.dp

private val QuickAccessItemTagSanitizer = Regex("[^A-Za-z0-9]+")

internal data class QuickAccessSlot(
    val activity: String,
    val index: Int,
    val topLeftPx: Offset,
    val widthPx: Int,
    val heightPx: Int
) {
    val centerPx: Offset
        get() = topLeftPx + Offset(widthPx / 2f, heightPx / 2f)
}

internal data class QuickAccessFlowLayoutResult(
    val slots: List<QuickAccessSlot>,
    val totalHeightPx: Int
)

internal fun <T> List<T>.moveItem(fromIndex: Int, toIndex: Int): List<T> {
    if (
        isEmpty() ||
        fromIndex !in indices ||
        toIndex !in indices ||
        fromIndex == toIndex
    ) {
        return this
    }
    val mutable = toMutableList()
    val moved = mutable.removeAt(fromIndex)
    mutable.add(toIndex, moved)
    return mutable.toList()
}

internal fun quickAccessGridTestTag(): String = "quick_access_grid_flow"

internal fun quickAccessItemTestTag(activity: String): String {
    val normalized = activity
        .trim()
        .replace(QuickAccessItemTagSanitizer, "_")
        .trim('_')
        .ifBlank { "blank" }
    return "quick_access_item_$normalized"
}

internal fun calculateQuickAccessFlowLayout(
    activities: List<String>,
    measuredWidthsPx: Map<String, Int>,
    containerWidthPx: Int,
    maxItemWidthPx: Int,
    itemHeightPx: Int,
    horizontalGapPx: Int,
    verticalGapPx: Int
): QuickAccessFlowLayoutResult {
    if (activities.isEmpty()) {
        return QuickAccessFlowLayoutResult(emptyList(), 0)
    }

    val safeContainerWidthPx = containerWidthPx.coerceAtLeast(1)
    val safeMaxItemWidthPx = maxItemWidthPx.coerceAtLeast(1)
    val safeItemHeightPx = itemHeightPx.coerceAtLeast(1)

    var x = 0
    var y = 0
    val slots = buildList {
        activities.forEachIndexed { index, activity ->
            val widthPx = measuredWidthsPx[activity]
                ?.coerceAtLeast(1)
                ?.coerceAtMost(safeMaxItemWidthPx)
                ?.coerceAtMost(safeContainerWidthPx)
                ?: safeMaxItemWidthPx.coerceAtMost(safeContainerWidthPx)
            if (x > 0 && x + widthPx > safeContainerWidthPx) {
                x = 0
                y += safeItemHeightPx + verticalGapPx
            }
            add(
                QuickAccessSlot(
                    activity = activity,
                    index = index,
                    topLeftPx = Offset(x.toFloat(), y.toFloat()),
                    widthPx = widthPx,
                    heightPx = safeItemHeightPx
                )
            )
            x += widthPx + horizontalGapPx
        }
    }

    return QuickAccessFlowLayoutResult(
        slots = slots,
        totalHeightPx = y + safeItemHeightPx
    )
}

internal fun calculateQuickAccessDropIndex(
    pointerCenter: Offset,
    slots: List<QuickAccessSlot>
): Int {
    if (slots.isEmpty()) {
        return -1
    }
    return slots.minBy { slot ->
        val dx = slot.centerPx.x - pointerCenter.x
        val dy = slot.centerPx.y - pointerCenter.y
        (dx * dx) + (dy * dy)
    }.index
}
