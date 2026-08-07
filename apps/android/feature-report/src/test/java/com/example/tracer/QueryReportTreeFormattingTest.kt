package com.example.tracer

import androidx.compose.ui.unit.dp
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class QueryReportTreeFormattingTest {
    @Test
    fun maxTreeDepth_usesTheDeepestVisibleNode() {
        val tree = listOf(
            TreeNode(
                name = "root",
                children = listOf(
                    TreeNode(
                        name = "level-1",
                        children = listOf(
                            TreeNode(
                                name = "level-2",
                                children = listOf(
                                    TreeNode(
                                        name = "level-3",
                                        children = listOf(
                                            TreeNode(name = "level-4")
                                        )
                                    )
                                )
                            )
                        )
                    )
                )
            )
        )

        assertEquals(4, tree.maxTreeDepth())
    }

    @Test
    fun treeContentMinWidth_growsForDeepHierarchies() {
        assertTrue(treeContentMinWidthForDepth(4) > treeContentMinWidthForDepth(0))
        assertTrue(treeContentMinWidthForDepth(5) > treeContentMinWidthForDepth(4))
    }
}
