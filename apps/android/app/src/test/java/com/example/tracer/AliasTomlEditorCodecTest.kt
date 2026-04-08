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
}
