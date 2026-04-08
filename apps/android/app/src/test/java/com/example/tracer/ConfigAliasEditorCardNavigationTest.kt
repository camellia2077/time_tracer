package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class ConfigAliasEditorCardNavigationTest {
    @Test
    fun resolve_layer_shows_only_target_level_nodes_for_deep_path() {
        // Contract: deep hierarchy should project to one current layer view.
        val definiteEntry = AliasTomlEntry(id = "entry-def", aliasKey = "变限积分", canonicalLeaf = "integrals")
        val definite = AliasTomlGroup(id = "g-def", name = "definite-integral", nodes = listOf(definiteEntry))
        val calculus = AliasTomlGroup(id = "g-cal", name = "calculus", nodes = listOf(definite))
        val math = AliasTomlGroup(id = "g-math", name = "math", nodes = listOf(calculus))
        val document = AliasTomlDocument(parent = "study", nodes = listOf(math))

        val layer = resolveAliasStructuredLayer(
            document = document,
            pathGroupIds = listOf("g-math", "g-cal", "g-def")
        )

        assertEquals(listOf("g-math", "g-cal", "g-def"), layer.normalizedPathGroupIds)
        assertEquals(listOf("math", "calculus", "definite-integral"), layer.breadcrumbs.map { it.name })
        assertTrue(layer.currentGroups.isEmpty())
        assertEquals(listOf("变限积分"), layer.currentEntries.map { it.aliasKey })
    }

    @Test
    fun resolve_layer_returns_child_groups_when_entering_parent_group() {
        // Contract: entering a group exposes its immediate children only.
        val calculus = AliasTomlGroup(id = "g-cal", name = "calculus")
        val algebra = AliasTomlGroup(id = "g-alg", name = "linear-algebra")
        val math = AliasTomlGroup(id = "g-math", name = "math", nodes = listOf(calculus, algebra))
        val document = AliasTomlDocument(parent = "study", nodes = listOf(math))

        val layer = resolveAliasStructuredLayer(
            document = document,
            pathGroupIds = listOf("g-math")
        )

        assertEquals(listOf("g-math"), layer.normalizedPathGroupIds)
        assertEquals(listOf("calculus", "linear-algebra"), layer.currentGroups.map { it.name })
        assertEquals("g-math", layer.currentParentGroupId)
    }

    @Test
    fun resolve_layer_falls_back_to_nearest_valid_ancestor_for_invalid_path() {
        // Contract: invalid path segments must snap back to nearest valid ancestor.
        val calculus = AliasTomlGroup(id = "g-cal", name = "calculus")
        val math = AliasTomlGroup(id = "g-math", name = "math", nodes = listOf(calculus))
        val document = AliasTomlDocument(parent = "study", nodes = listOf(math))

        val layer = resolveAliasStructuredLayer(
            document = document,
            pathGroupIds = listOf("g-math", "missing", "g-cal")
        )

        assertEquals(listOf("g-math"), layer.normalizedPathGroupIds)
        assertEquals(listOf("calculus"), layer.currentGroups.map { it.name })
    }

    @Test
    fun resolve_layer_uses_root_nodes_when_path_is_empty() {
        // Contract: empty path always maps to root layer.
        val rootEntry = AliasTomlEntry(id = "entry-root", aliasKey = "英语", canonicalLeaf = "english")
        val rootGroup = AliasTomlGroup(id = "g-math", name = "math")
        val document = AliasTomlDocument(parent = "study", nodes = listOf(rootGroup, rootEntry))

        val layer = resolveAliasStructuredLayer(
            document = document,
            pathGroupIds = emptyList()
        )

        assertTrue(layer.normalizedPathGroupIds.isEmpty())
        assertEquals(listOf("math"), layer.currentGroups.map { it.name })
        assertEquals(listOf("英语"), layer.currentEntries.map { it.aliasKey })
        assertEquals(null, layer.currentParentGroupId)
    }
}
