package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Assert.assertNotNull
import org.junit.Assert.assertTrue
import org.junit.Test

class AliasTomlEditorCodecTest {
    @Test
    fun parse_study_style_nested_groups_into_tree() {
        val result = AliasTomlEditorCodec.parse(
            """
            parent = "study"

            [aliases.english]
            "英语单词" = "words"

            [aliases.math]
            "高等数学" = "calculus"

            [aliases.math.calculus]
            "高等数学二重积分" = "double-integral"
            """.trimIndent()
        )

        val document = result.document
        assertNotNull(document)
        requireNotNull(document)
        assertEquals("study", document.parent)
        assertEquals(2, document.nodes.size)
        val mathGroup = document.nodes.filterIsInstance<AliasTomlGroup>().single { it.name == "math" }
        val calculusGroup = mathGroup.nodes.filterIsInstance<AliasTomlGroup>().single { it.name == "calculus" }
        val nestedEntry = calculusGroup.nodes.filterIsInstance<AliasTomlEntry>().single()
        assertEquals("高等数学二重积分", nestedEntry.aliasKey)
        assertEquals("double-integral", nestedEntry.canonicalLeaf)
    }

    @Test
    fun parse_recreation_style_shallow_groups() {
        val result = AliasTomlEditorCodec.parse(
            """
            parent = "recreation"

            [aliases.online-platforms]
            "zhihu" = "zhihu"

            [aliases.game]
            "minecraft" = "minecraft"
            """.trimIndent()
        )

        val document = result.document
        assertNotNull(document)
        requireNotNull(document)
        assertEquals("recreation", document.parent)
        assertEquals(2, document.nodes.filterIsInstance<AliasTomlGroup>().size)
    }

    @Test
    fun parse_rejects_missing_parent() {
        val result = AliasTomlEditorCodec.parse(
            """
            [aliases.study]
            "英语单词" = "words"
            """.trimIndent()
        )

        assertTrue(result.errorMessage.contains("parent"))
    }

    @Test
    fun parse_rejects_missing_aliases_table() {
        val result = AliasTomlEditorCodec.parse(
            """
            parent = "study"
            """.trimIndent()
        )

        assertTrue(result.errorMessage.contains("aliases"))
    }

    @Test
    fun parse_rejects_empty_alias_key_and_empty_leaf() {
        val emptyAliasKey = AliasTomlEditorCodec.parse(
            """
            parent = "study"

            [aliases.english]
            "" = "words"
            """.trimIndent()
        )
        val emptyLeaf = AliasTomlEditorCodec.parse(
            """
            parent = "study"

            [aliases.english]
            "英语单词" = ""
            """.trimIndent()
        )

        assertTrue(emptyAliasKey.errorMessage.contains("empty alias key"))
        assertTrue(emptyLeaf.errorMessage.contains("non-empty canonical leaf"))
    }

    @Test
    fun serialize_writes_nested_alias_sections() {
        val document = AliasTomlDocument(
            parent = "study",
            nodes = listOf(
                AliasTomlGroup(
                    name = "math",
                    nodes = listOf(
                        AliasTomlEntry(aliasKey = "高等数学", canonicalLeaf = "calculus"),
                        AliasTomlGroup(
                            name = "linear-algebra",
                            nodes = listOf(
                                AliasTomlEntry(aliasKey = "线性代数矩阵", canonicalLeaf = "matrix")
                            )
                        )
                    )
                )
            )
        )

        val serialized = AliasTomlEditorCodec.serialize(document)

        assertTrue(serialized.contains("[aliases.math]"))
        assertTrue(serialized.contains("[aliases.math.linear-algebra]"))
        val reparsed = AliasTomlEditorCodec.parse(serialized)
        assertTrue(reparsed.errorMessage.isEmpty())
        assertEquals(listOf("高等数学", "线性代数矩阵"), AliasTomlEditorCodec.collectAliasKeys(requireNotNull(reparsed.document)))
    }

    @Test
    fun validate_rejects_empty_leaf_and_duplicate_group_name() {
        val document = AliasTomlDocument(
            parent = "study",
            nodes = listOf(
                AliasTomlGroup(name = "math"),
                AliasTomlGroup(name = "math"),
                AliasTomlEntry(aliasKey = "bad", canonicalLeaf = "")
            )
        )

        val message = AliasTomlEditorCodec.validateForSave(document)

        assertNotNull(message)
        requireNotNull(message)
        assertTrue(message.contains("duplicate child group") || message.contains("non-empty canonical leaf"))
    }

    @Test
    fun plan_leaf_move_previews_changed_canonical_without_mutating_document() {
        val walking = AliasTomlEntry(id = "entry-walking", aliasKey = "散步", canonicalLeaf = "walking")
        val cardio = AliasTomlGroup(id = "group-cardio", name = "cardio")
        val document = AliasTomlDocument(
            parent = "exercise",
            nodes = listOf(cardio, walking)
        )

        val result = document.planEntryMove(
            entryId = walking.id,
            targetGroupId = cardio.id
        )

        val plan = (result as? AliasEntryMovePlanResult.Ready)?.plan
        assertNotNull(plan)
        requireNotNull(plan)
        assertEquals("exercise_walking", plan.oldCanonical)
        assertEquals("exercise_cardio_walking", plan.newCanonical)
        assertEquals(listOf("cardio"), plan.targetGroupPath)
        // Planning must leave the original structured TOML draft intact.
        assertEquals(listOf(cardio, walking), document.nodes)
    }

    @Test
    fun apply_entry_move_plan_reparents_only_the_leaf_in_memory() {
        val climbing = AliasTomlEntry(id = "entry-climbing", aliasKey = "爬坡", canonicalLeaf = "climb")
        val cardio = AliasTomlGroup(id = "group-cardio", name = "cardio")
        val document = AliasTomlDocument(
            parent = "exercise",
            nodes = listOf(cardio, climbing)
        )
        val plan = (document.planEntryMove(climbing.id, cardio.id) as AliasEntryMovePlanResult.Ready).plan

        val moved = document.applyEntryMovePlan(plan)

        assertTrue(moved.nodes.none { node -> node is AliasTomlEntry && node.id == climbing.id })
        val movedCardio = moved.nodes.filterIsInstance<AliasTomlGroup>().single()
        assertEquals(listOf(climbing), movedCardio.nodes)
    }

    @Test
    fun plan_entry_move_rejects_same_parent_and_canonical_collisions() {
        val walking = AliasTomlEntry(id = "entry-walking", aliasKey = "散步", canonicalLeaf = "walking")
        val cardioWalking = AliasTomlEntry(id = "entry-cardio-walking", aliasKey = "快走", canonicalLeaf = "walking")
        val cardio = AliasTomlGroup(
            id = "group-cardio",
            name = "cardio",
            nodes = listOf(cardioWalking)
        )
        val document = AliasTomlDocument(
            parent = "exercise",
            nodes = listOf(cardio, walking)
        )

        val collision = document.planEntryMove(walking.id, cardio.id)
        val sameParent = document.planEntryMove(cardioWalking.id, cardio.id)

        assertTrue((collision as AliasEntryMovePlanResult.Invalid).message.contains("collide"))
        assertTrue((sameParent as AliasEntryMovePlanResult.Invalid).message.contains("already"))
    }

    @Test
    fun promote_leaf_to_recordable_group_preserves_its_canonical_path() {
        val online = AliasTomlEntry(id = "entry-online", aliasKey = "上网", canonicalLeaf = "online")
        val document = AliasTomlDocument(parent = "recreation", nodes = listOf(online))

        val plan = (document.planEntryPromotion(online.id) as AliasEntryPromotePlanResult.Ready).plan
        val promoted = document.applyEntryPromotePlan(plan)
        val group = promoted.nodes.filterIsInstance<AliasTomlGroup>().single()

        assertEquals("recreation_online", plan.preservedCanonical)
        assertEquals("online", group.name)
        assertEquals(listOf("上网"), group.groupAliases)
        val serialized = AliasTomlEditorCodec.serialize(promoted)
        assertTrue(serialized.contains("group_aliases = [\"上网\"]"))
        assertEquals(listOf("上网"), AliasTomlEditorCodec.collectAliasKeys(
            requireNotNull(AliasTomlEditorCodec.parse(serialized).document)
        ))
    }

    @Test
    fun rename_group_alias_updates_only_the_existing_recordable_name() {
        val group = AliasTomlGroup(
            id = "group-online",
            name = "online",
            groupAliases = listOf("上网"),
            nodes = listOf(AliasTomlEntry(aliasKey = "哔哩哔哩", canonicalLeaf = "bilibili"))
        )
        val document = AliasTomlDocument(parent = "recreation", nodes = listOf(group))

        val updated = document.renameGroupAlias(group.id, "上网", "网上活动")
        val updatedGroup = updated.nodes.filterIsInstance<AliasTomlGroup>().single()

        assertEquals(listOf("网上活动"), updatedGroup.groupAliases)
        assertEquals("哔哩哔哩", updatedGroup.nodes.filterIsInstance<AliasTomlEntry>().single().aliasKey)
        assertTrue(AliasTomlEditorCodec.serialize(updated).contains("group_aliases = [\"网上活动\"]"))
    }

    @Test
    fun add_group_alias_keeps_existing_record_names() {
        val group = AliasTomlGroup(id = "group-online", name = "online", groupAliases = listOf("上网"))
        val updated = AliasTomlDocument(parent = "recreation", nodes = listOf(group))
            .addGroupAlias(group.id, "浏览网页")

        assertEquals(
            listOf("上网", "浏览网页"),
            updated.nodes.filterIsInstance<AliasTomlGroup>().single().groupAliases
        )
    }
}
