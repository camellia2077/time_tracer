package com.example.tracer

import kotlinx.coroutines.runBlocking
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test
import java.io.File
import java.nio.file.Files

class RuntimeCanonicalCatalogTest {
    @Test
    fun canonicalCatalogParser_parsesNestedAliasGroups() {
        val parseResult = RuntimeCanonicalCatalogParser.parse(
            """
                parent = "study"

                [aliases.math]
                "calculus-overview" = ["高等数学"]

                [aliases.math.calculus]
                "double-integral" = ["高等数学二重积分"]
            """.trimIndent()
        )

        val document = requireNotNull(parseResult.document)
        assertEquals("study", document.parent)
        val mathGroup = document.nodes.single() as CanonicalAliasGroup
        assertEquals("math", mathGroup.name)
        assertEquals("calculus-overview", (mathGroup.nodes[0] as CanonicalAliasEntry).canonicalLeaf)
        val calculusGroup = mathGroup.nodes[1] as CanonicalAliasGroup
        assertEquals("calculus", calculusGroup.name)
        assertEquals(
            "double-integral",
            (calculusGroup.nodes.single() as CanonicalAliasEntry).canonicalLeaf
        )
    }

    @Test
    fun canonicalCatalogParser_supportsRecordableGroupAliases() {
        val parseResult = RuntimeCanonicalCatalogParser.parse(
            """
                parent = "recreation"

                [aliases.online]
                group_aliases = ["上网"]
                "bilibili" = ["哔哩哔哩"]
            """.trimIndent()
        )

        val online = requireNotNull(parseResult.document).nodes.single() as CanonicalAliasGroup
        assertEquals(listOf("上网"), online.groupAliases)
        val catalog = RuntimeCanonicalCatalogBuilder.build(listOf("online.toml" to requireNotNull(parseResult.document)))
        assertEquals(listOf("recreation_online", "recreation_online_bilibili"), catalog.entries.map { it.canonicalPath })
        assertEquals(listOf("上网"), catalog.entries.first().aliases)
    }

    @Test
    fun listCanonicalCatalog_buildsPathTreeAndDeduplicatesAliases() = runBlocking {
        val root = Files.createTempDirectory("runtime-canonical-catalog").toFile()
        try {
            writeAliasToml(
                root = root,
                relativePath = "activity_hierarchy/meal.toml",
                content = """
                    parent = "meal"

                    [aliases]
                    "dining" = ["meal", "吃饭"]
                    "breakfast" = ["早餐"]
                """.trimIndent() + "\n"
            )
            writeAliasToml(
                root = root,
                relativePath = "activity_hierarchy/study.toml",
                content = """
                    parent = "study"

                    [aliases.math]
                    "calculus-overview" = ["高等数学"]

                    [aliases.math.calculus]
                    "double-integral" = ["高等数学二重积分"]
                """.trimIndent() + "\n"
            )

            val result = RuntimeCanonicalCatalogQueryDelegate(
                ensureConfigTomlStorage = { ConfigTomlStorage(root.absolutePath) }
            ).listCanonicalCatalog()

            assertTrue(result.ok)
            assertEquals(4, result.entries.size)
            assertEquals(
                listOf("meal", "study"),
                result.roots.map { it.path }
            )

            val mealRoot = result.roots.first { it.path == "meal" }
            assertEquals(
                listOf("meal_breakfast", "meal_dining"),
                mealRoot.entries.map { it.canonicalPath }
            )
            assertEquals(
                listOf("meal", "吃饭"),
                mealRoot.entries.first { it.canonicalPath == "meal_dining" }.aliases
            )

            val studyRoot = result.roots.first { it.path == "study" }
            assertEquals(listOf("study_math"), studyRoot.children.map { it.path })
            val mathNode = studyRoot.children.single()
            assertEquals(listOf("study_math_calculus-overview"), mathNode.entries.map { it.canonicalPath })
            val calculusNode = mathNode.children.single()
            assertEquals("study_math_calculus", calculusNode.path)
            assertEquals(
                listOf("study_math_calculus_double-integral"),
                calculusNode.entries.map { it.canonicalPath }
            )
        } finally {
            root.deleteRecursively()
        }
    }

    @Test
    fun listCanonicalCatalog_skipsTheFixedSystemAliasConfig() = runBlocking {
        val root = Files.createTempDirectory("runtime-canonical-catalog-system").toFile()
        try {
            writeAliasToml(
                root = root,
                relativePath = "activity_hierarchy/_system.toml",
                content = """
                    [sleep_inference]
                    wake_keywords = ["起床"]
                """.trimIndent()
            )
            writeAliasToml(
                root = root,
                relativePath = "activity_hierarchy/other.toml",
                content = """
                    parent = "other"

                    [aliases]
                    "looking-for" = ["找东西"]
                """.trimIndent()
            )

            val result = RuntimeCanonicalCatalogQueryDelegate(
                ensureConfigTomlStorage = { ConfigTomlStorage(root.absolutePath) }
            ).listCanonicalCatalog()

            assertTrue(result.ok)
            assertEquals(listOf("other_looking-for"), result.entries.map { it.canonicalPath })
        } finally {
            root.deleteRecursively()
        }
    }

    @Test
    fun listCanonicalCatalog_invalidAliasFileReturnsFailure() = runBlocking {
        val root = Files.createTempDirectory("runtime-canonical-catalog-invalid").toFile()
        try {
            writeAliasToml(
                root = root,
                relativePath = "activity_hierarchy/broken.toml",
                content = """
                    [aliases]
                    "dining" = ["meal"]
                """.trimIndent() + "\n"
            )

            val result = RuntimeCanonicalCatalogQueryDelegate(
                ensureConfigTomlStorage = { ConfigTomlStorage(root.absolutePath) }
            ).listCanonicalCatalog()

            assertFalse(result.ok)
            assertTrue(result.message.contains("broken.toml"))
        } finally {
            root.deleteRecursively()
        }
    }

    private fun writeAliasToml(root: File, relativePath: String, content: String) {
        File(root, relativePath).apply {
            parentFile?.mkdirs()
            writeText(content)
        }
    }
}
