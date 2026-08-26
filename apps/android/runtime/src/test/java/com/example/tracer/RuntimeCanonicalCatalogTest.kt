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
    fun canonicalCatalogBuilder_buildsNestedCoreSnapshots() {
        val catalog = RuntimeCanonicalCatalogBuilder.buildFromSnapshots(
            listOf(
                "online.toml" to hierarchy(
                    parent = "recreation",
                    nodes = listOf(
                        groupNode(
                            canonicalKey = "online",
                            aliases = listOf("上网"),
                            children = listOf(leafNode("bilibili", "哔哩哔哩"))
                        )
                    )
                )
            )
        )
        assertEquals(listOf("recreation_online", "recreation_online_bilibili"), catalog.entries.map { it.canonicalPath })
        assertEquals(listOf("上网"), catalog.entries.first().aliases)
    }

    @Test
    fun listCanonicalCatalog_buildsPathTreeAndDeduplicatesAliases() = runBlocking {
        val root = Files.createTempDirectory("runtime-canonical-catalog").toFile()
        try {
            writeAliasToml(
                root = root,
                relativePath = "user/activity_hierarchy/meal.toml",
                content = """
                    parent = "meal"

                    [canonical]
                    "dining" = ["meal", "吃饭"]
                    "breakfast" = ["早餐"]
                """.trimIndent() + "\n"
            )
            writeAliasToml(
                root = root,
                relativePath = "user/activity_hierarchy/study.toml",
                content = """
                    parent = "study"

                    [canonical.math]
                    "calculus-overview" = ["高等数学"]

                    [canonical.math.calculus]
                    "double-integral" = ["高等数学二重积分"]
                """.trimIndent() + "\n"
            )

            val result = RuntimeCanonicalCatalogQueryDelegate(
                ensureConfigTomlStorage = { ConfigTomlStorage(root.absolutePath) },
                searchActivityHierarchy = { content, _ ->
                    when {
                        content.contains("parent = \"meal\"") -> hierarchyResult(
                            "meal",
                            listOf(leafNode("dining", "meal", "吃饭"), leafNode("breakfast", "早餐"))
                        )

                        content.contains("parent = \"study\"") -> hierarchyResult(
                            "study",
                            listOf(
                                groupNode(
                                    canonicalKey = "math",
                                    children = listOf(
                                        leafNode("calculus-overview", "高等数学"),
                                        groupNode(
                                            canonicalKey = "calculus",
                                            children = listOf(leafNode("double-integral", "高等数学二重积分"))
                                        )
                                    )
                                )
                            )
                        )

                        else -> ActivityHierarchyDescribeResult(ok = false, message = "unexpected test TOML")
                    }
                }
            ).listCanonicalCatalog("")

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
                relativePath = "user/behavior.toml",
                content = """
                    [sleep_inference]
                    wake_keywords = ["起床"]
                """.trimIndent()
            )
            writeAliasToml(
                root = root,
                relativePath = "user/activity_hierarchy/other.toml",
                content = """
                    parent = "other"

                    [canonical]
                    "looking-for" = ["找东西"]
                """.trimIndent()
            )

            val result = RuntimeCanonicalCatalogQueryDelegate(
                ensureConfigTomlStorage = { ConfigTomlStorage(root.absolutePath) },
                searchActivityHierarchy = { _, _ ->
                    hierarchyResult("other", listOf(leafNode("looking-for", "找东西")))
                }
            ).listCanonicalCatalog("")

            assertTrue(result.ok)
            assertEquals(listOf("other_looking-for"), result.entries.map { it.canonicalPath })
        } finally {
            root.deleteRecursively()
        }
    }

    @Test
    fun listCanonicalCatalog_searchUsesCoreFilteredHierarchy() = runBlocking {
        val root = Files.createTempDirectory("runtime-canonical-catalog-search").toFile()
        try {
            writeAliasToml(
                root = root,
                relativePath = "user/activity_hierarchy/exercise.toml",
                content = "parent = \"exercise\"\n"
            )

            var receivedQuery = ""
            val result = RuntimeCanonicalCatalogQueryDelegate(
                ensureConfigTomlStorage = { ConfigTomlStorage(root.absolutePath) },
                searchActivityHierarchy = { _, query ->
                    receivedQuery = query
                    ActivityHierarchyDescribeResult(
                        ok = true,
                        hierarchy = ActivityHierarchySnapshot(
                            parent = "exercise",
                            nodes = listOf(
                                ActivityHierarchyNode(
                                    canonicalKey = "cardio",
                                    path = "cardio",
                                    kind = ActivityHierarchyNodeKind.GROUP,
                                    aliases = emptyList(),
                                    children = listOf(
                                        ActivityHierarchyNode(
                                            canonicalKey = "treadmill",
                                            path = "cardio.treadmill",
                                            kind = ActivityHierarchyNodeKind.LEAF,
                                            aliases = listOf("跑步机"),
                                            children = emptyList()
                                        )
                                    )
                                )
                            )
                        )
                    )
                }
            ).listCanonicalCatalog("跑步机")

            assertEquals("跑步机", receivedQuery)
            assertTrue(result.ok)
            assertEquals(listOf("exercise_cardio_treadmill"), result.entries.map { it.canonicalPath })
            assertEquals(listOf("跑步机"), result.entries.single().aliases)
        } finally {
            root.deleteRecursively()
        }
    }

    @Test
    fun canonicalCatalogBuilder_skipsParentsWithNoSearchMatches() {
        val matchingLeaf = ActivityHierarchyNode(
            canonicalKey = "treadmill",
            path = "treadmill",
            kind = ActivityHierarchyNodeKind.LEAF,
            aliases = listOf("跑步机"),
            children = emptyList()
        )

        val result = RuntimeCanonicalCatalogBuilder.buildFromSnapshots(
            listOf(
                "exercise.toml" to ActivityHierarchySnapshot(
                    parent = "exercise",
                    nodes = listOf(matchingLeaf)
                ),
                "study.toml" to ActivityHierarchySnapshot(
                    parent = "study",
                    nodes = emptyList()
                )
            )
        )

        assertEquals(listOf("exercise"), result.roots.map { it.path })
        assertEquals(listOf("exercise_treadmill"), result.entries.map { it.canonicalPath })
    }

    @Test
    fun listCanonicalCatalog_invalidAliasFileReturnsFailure() = runBlocking {
        val root = Files.createTempDirectory("runtime-canonical-catalog-invalid").toFile()
        try {
            writeAliasToml(
                root = root,
                relativePath = "user/activity_hierarchy/broken.toml",
                content = """
                    [canonical]
                    "dining" = ["meal"]
                """.trimIndent() + "\n"
            )

            val result = RuntimeCanonicalCatalogQueryDelegate(
                ensureConfigTomlStorage = { ConfigTomlStorage(root.absolutePath) },
                searchActivityHierarchy = { _, _ ->
                    ActivityHierarchyDescribeResult(ok = false, message = "invalid hierarchy")
                }
            ).listCanonicalCatalog("")

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

    private fun hierarchyResult(
        parent: String,
        nodes: List<ActivityHierarchyNode>
    ): ActivityHierarchyDescribeResult = ActivityHierarchyDescribeResult(
        ok = true,
        hierarchy = hierarchy(parent, nodes)
    )

    private fun hierarchy(
        parent: String,
        nodes: List<ActivityHierarchyNode>
    ) = ActivityHierarchySnapshot(parent = parent, nodes = nodes)

    private fun leafNode(canonicalKey: String, vararg aliases: String) = ActivityHierarchyNode(
        canonicalKey = canonicalKey,
        path = canonicalKey,
        kind = ActivityHierarchyNodeKind.LEAF,
        aliases = aliases.toList(),
        children = emptyList()
    )

    private fun groupNode(
        canonicalKey: String,
        aliases: List<String> = emptyList(),
        children: List<ActivityHierarchyNode>
    ) = ActivityHierarchyNode(
        canonicalKey = canonicalKey,
        path = canonicalKey,
        kind = ActivityHierarchyNodeKind.GROUP,
        aliases = aliases,
        children = children
    )
}
