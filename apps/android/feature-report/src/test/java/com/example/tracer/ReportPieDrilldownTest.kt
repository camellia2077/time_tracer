package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test

class ReportPieDrilldownTest {
    private val tree = listOf(
        TreeNode(
            name = "study",
            durationSeconds = 7_200L,
            children = listOf(
                TreeNode(
                    name = "math",
                    durationSeconds = 3_600L,
                    children = listOf(
                        TreeNode(name = "algebra", durationSeconds = 3_600L)
                    )
                ),
                TreeNode(name = "reading", durationSeconds = 3_600L)
            )
        ),
        TreeNode(name = "sleep", durationSeconds = 28_800L)
    )

    @Test
    fun resolveCompositionDrilldownNodes_entersEachSelectedTreeLevel() {
        assertEquals(listOf("study", "sleep"), resolveCompositionDrilldownNodes(tree, emptyList()).map(TreeNode::name))
        assertEquals(listOf("math", "reading"), resolveCompositionDrilldownNodes(tree, listOf("study")).map(TreeNode::name))
        assertEquals(listOf("algebra"), resolveCompositionDrilldownNodes(tree, listOf("study", "math")).map(TreeNode::name))
    }

    @Test
    fun resolveCompositionDrilldownNodes_invalidPath_returnsRootLevel() {
        assertEquals(listOf("study", "sleep"), resolveCompositionDrilldownNodes(tree, listOf("missing")).map(TreeNode::name))
    }

    @Test
    fun toReportCompositionSlices_derivesRankedSlicesFromEveryTreeLevel() {
        val rootSlices = tree.toReportCompositionSlices()
        val childSlices = resolveCompositionDrilldownNodes(tree, listOf("study"))
            .toReportCompositionSlices()

        assertEquals(listOf("sleep", "study"), rootSlices.map(ReportCompositionSlice::root))
        assertEquals(listOf("math", "reading"), childSlices.map(ReportCompositionSlice::root))
        assertEquals(80f, rootSlices.first().percent)
    }

    @Test
    fun toReportCompositionSlices_frequency_usesCurrentLevelOccurrenceCounts() {
        val nodes = listOf(
            TreeNode(name = "study", durationSeconds = 7_200L, occurrenceCount = 2L),
            TreeNode(name = "sleep", durationSeconds = 3_600L, occurrenceCount = 6L)
        )

        val slices = nodes.toReportCompositionSlices(ReportCompositionMeasure.FREQUENCY)

        assertEquals(listOf("sleep", "study"), slices.map(ReportCompositionSlice::root))
        assertEquals(listOf(6L, 2L), slices.map(ReportCompositionSlice::durationSeconds))
        assertEquals(75f, slices.first().percent)
    }

    @Test
    fun shouldDrawInlinePieLabel_onlyAllowsLargeManageableSlices() {
        assertTrue(shouldDrawInlinePieLabel(sweepAngle = 44f, sliceCount = 6))
        assertFalse(shouldDrawInlinePieLabel(sweepAngle = 43f, sliceCount = 6))
        assertFalse(shouldDrawInlinePieLabel(sweepAngle = 80f, sliceCount = 9))
    }
}
