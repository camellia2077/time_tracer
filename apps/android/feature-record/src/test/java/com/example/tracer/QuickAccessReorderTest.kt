package com.example.tracer

import androidx.compose.ui.geometry.Offset
import org.junit.Assert.assertEquals
import org.junit.Test

class QuickAccessReorderTest {
    @Test
    fun moveItem_returnsOriginalList_whenIndexesAreTheSame() {
        val activities = listOf("meal", "wash", "sleep")

        val reordered = activities.moveItem(fromIndex = 1, toIndex = 1)

        assertEquals(activities, reordered)
    }

    @Test
    fun moveItem_returnsOriginalList_whenIndexesAreInvalid() {
        val activities = listOf("meal", "wash", "sleep")

        val reordered = activities.moveItem(fromIndex = -1, toIndex = 2)

        assertEquals(activities, reordered)
    }

    @Test
    fun moveItem_movesFirstItemToEnd() {
        val activities = listOf("meal", "wash", "sleep")

        val reordered = activities.moveItem(fromIndex = 0, toIndex = 2)

        assertEquals(listOf("wash", "sleep", "meal"), reordered)
    }

    @Test
    fun moveItem_movesAcrossGridRows() {
        val activities = listOf("meal", "wash", "sleep", "study", "walk")

        val reordered = activities.moveItem(fromIndex = 4, toIndex = 1)

        assertEquals(listOf("meal", "walk", "wash", "sleep", "study"), reordered)
    }

    @Test
    fun calculateQuickAccessFlowLayout_wrapsMixedWidthsAcrossRows() {
        val layout = calculateQuickAccessFlowLayout(
            activities = listOf("bilibili", "zhihu", "weibo"),
            measuredWidthsPx = mapOf(
                "bilibili" to 150,
                "zhihu" to 100,
                "weibo" to 100
            ),
            containerWidthPx = 260,
            maxItemWidthPx = 240,
            itemHeightPx = 40,
            horizontalGapPx = 8,
            verticalGapPx = 8
        )

        assertEquals(3, layout.slots.size)
        assertEquals(Offset(0f, 0f), layout.slots[0].topLeftPx)
        assertEquals(Offset(158f, 0f), layout.slots[1].topLeftPx)
        assertEquals(Offset(0f, 48f), layout.slots[2].topLeftPx)
        assertEquals(88, layout.totalHeightPx)
    }

    @Test
    fun calculateQuickAccessFlowLayout_clampsWidthToMaxAndContainer() {
        val layout = calculateQuickAccessFlowLayout(
            activities = listOf("very_long_activity_name"),
            measuredWidthsPx = mapOf("very_long_activity_name" to 420),
            containerWidthPx = 180,
            maxItemWidthPx = 240,
            itemHeightPx = 40,
            horizontalGapPx = 8,
            verticalGapPx = 8
        )

        assertEquals(1, layout.slots.size)
        assertEquals(180, layout.slots.single().widthPx)
        assertEquals(40, layout.totalHeightPx)
    }

    @Test
    fun calculateQuickAccessDropIndex_returnsClosestSlotAcrossRows() {
        val slots = calculateQuickAccessFlowLayout(
            activities = listOf("bilibili", "zhihu", "weibo"),
            measuredWidthsPx = mapOf(
                "bilibili" to 150,
                "zhihu" to 100,
                "weibo" to 100
            ),
            containerWidthPx = 260,
            maxItemWidthPx = 240,
            itemHeightPx = 40,
            horizontalGapPx = 8,
            verticalGapPx = 8
        ).slots

        val targetIndex = calculateQuickAccessDropIndex(
            pointerCenter = Offset(x = 40f, y = 70f),
            slots = slots
        )

        assertEquals(2, targetIndex)
    }
}
