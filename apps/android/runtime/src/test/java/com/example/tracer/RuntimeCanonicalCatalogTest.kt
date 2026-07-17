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
                "高等数学" = "calculus"

                [aliases.math.calculus]
                "高等数学二重积分" = "double-integral"
            """.trimIndent()
        )

        val document = requireNotNull(parseResult.document)
        assertEquals("study", document.parent)
        val mathGroup = document.nodes.single() as CanonicalAliasGroup
        assertEquals("math", mathGroup.name)
        assertEquals("calculus", (mathGroup.nodes[0] as CanonicalAliasEntry).canonicalLeaf)
        val calculusGroup = mathGroup.nodes[1] as CanonicalAliasGroup
        assertEquals("calculus", calculusGroup.name)
        assertEquals(
            "double-integral",
            (calculusGroup.nodes.single() as CanonicalAliasEntry).canonicalLeaf
        )
    }

    @Test
    fun listCanonicalCatalog_buildsPathTreeAndDeduplicatesAliases() = runBlocking {
        val root = Files.createTempDirectory("runtime-canonical-catalog").toFile()
        try {
            writeAliasToml(
                root = root,
                relativePath = "converter/aliases/meal.toml",
                content = """
                    parent = "meal"

                    [aliases]
                    "meal" = "dining"
                    "吃饭" = "dining"
                    "早餐" = "breakfast"
                """.trimIndent() + "\n"
            )
            writeAliasToml(
                root = root,
                relativePath = "converter/aliases/study.toml",
                content = """
                    parent = "study"

                    [aliases.math]
                    "高等数学" = "calculus"

                    [aliases.math.calculus]
                    "高等数学二重积分" = "double-integral"
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
                listOf("meal/breakfast", "meal/dining"),
                mealRoot.entries.map { it.canonicalPath }
            )
            assertEquals(
                listOf("meal", "吃饭"),
                mealRoot.entries.first { it.canonicalPath == "meal/dining" }.aliases
            )

            val studyRoot = result.roots.first { it.path == "study" }
            assertEquals(listOf("study/math"), studyRoot.children.map { it.path })
            val mathNode = studyRoot.children.single()
            assertEquals(listOf("study/math/calculus"), mathNode.entries.map { it.canonicalPath })
            val calculusNode = mathNode.children.single()
            assertEquals("study/math/calculus", calculusNode.path)
            assertEquals(
                listOf("study/math/calculus/double-integral"),
                calculusNode.entries.map { it.canonicalPath }
            )
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
                relativePath = "converter/aliases/broken.toml",
                content = """
                    [aliases]
                    "meal" = "dining"
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
