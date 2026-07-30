package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test
import java.io.File
import java.nio.file.Files

class ConfigTomlStorageTest {
    @Test
    fun readTomlFile_canonicalizesLegacyUtf8Text() {
        val root = Files.createTempDirectory("config-toml-storage").toFile()
        try {
            val target = File(root, "program/meta/bundle.toml")
            target.parentFile?.mkdirs()
            target.writeBytes(
                byteArrayOf(
                    0xEF.toByte(),
                    0xBB.toByte(),
                    0xBF.toByte(),
                    's'.code.toByte(),
                    'c'.code.toByte(),
                    'h'.code.toByte(),
                    'e'.code.toByte(),
                    'm'.code.toByte(),
                    'a'.code.toByte(),
                    '_'.code.toByte(),
                    'v'.code.toByte(),
                    'e'.code.toByte(),
                    'r'.code.toByte(),
                    's'.code.toByte(),
                    'i'.code.toByte(),
                    'o'.code.toByte(),
                    'n'.code.toByte(),
                    ' '.code.toByte(),
                    '='.code.toByte(),
                    ' '.code.toByte(),
                    '1'.code.toByte(),
                    '\r'.code.toByte(),
                    '\n'.code.toByte()
                )
            )

            val result = ConfigTomlStorage(root.absolutePath).readTomlFile("program/meta/bundle.toml")

            assertTrue(result.ok)
            assertEquals("schema_version = 1\n", result.content)
        } finally {
            root.deleteRecursively()
        }
    }

    @Test
    fun writeTomlFile_rewritesMutableHierarchyContentAsCanonicalUtf8() {
        val root = Files.createTempDirectory("config-toml-storage-write").toFile()
        try {
            val target = File(root, "user/activity_hierarchy/custom.toml")
            target.parentFile?.mkdirs()
            target.writeText("seed")

            val result = ConfigTomlStorage(root.absolutePath).writeTomlFile(
                relativePath = "user/activity_hierarchy/custom.toml",
                content = "\uFEFFprofile = \"android\"\r\n"
            )

            assertTrue(result.ok)
            assertEquals("profile = \"android\"\n", target.readText())
        } finally {
            root.deleteRecursively()
        }
    }

    @Test
    fun writeTomlFile_createsMissingTomlFileUnderConfigRoot() {
        val root = Files.createTempDirectory("config-toml-storage-create").toFile()
        try {
            val target = File(root, "user/activity_hierarchy/custom.toml")

            val result = ConfigTomlStorage(root.absolutePath).writeTomlFile(
                relativePath = "user/activity_hierarchy/custom.toml",
                content = "name = \"custom\"\r\n"
            )

            assertTrue(result.ok)
            assertEquals("name = \"custom\"\n", target.readText())
        } finally {
            root.deleteRecursively()
        }
    }

    @Test
    fun writeTomlFile_rejectsProgramResourcePaths() {
        val root = Files.createTempDirectory("config-toml-storage-program-read-only").toFile()
        try {
            val target = File(root, "program/charts/heatmap.toml").apply {
                parentFile?.mkdirs()
                writeText("seed")
            }

            val result = ConfigTomlStorage(root.absolutePath).writeTomlFile(
                relativePath = "program/charts/heatmap.toml",
                content = "changed = true"
            )

            assertFalse(result.ok)
            assertTrue(result.message.contains("read-only"))
            assertEquals("seed", target.readText())
        } finally {
            root.deleteRecursively()
        }
    }

    @Test
    fun deleteTomlFile_rejectsProgramResourcePaths() {
        val root = Files.createTempDirectory("config-toml-storage-program-delete").toFile()
        try {
            val target = File(root, "program/reports/markdown/en/day.toml").apply {
                parentFile?.mkdirs()
                writeText("seed")
            }

            val result = ConfigTomlStorage(root.absolutePath).deleteTomlFile(
                relativePath = "program/reports/markdown/en/day.toml"
            )

            assertFalse(result.ok)
            assertTrue(result.message.contains("read-only"))
            assertTrue(target.exists())
        } finally {
            root.deleteRecursively()
        }
    }

    @Test
    fun readTomlFile_invalidUtf8ReturnsFailure() {
        val root = Files.createTempDirectory("config-toml-storage-invalid").toFile()
        try {
            val target = File(root, "program/meta/bundle.toml")
            target.parentFile?.mkdirs()
            target.writeBytes(byteArrayOf(0xFF.toByte()))

            val result = ConfigTomlStorage(root.absolutePath).readTomlFile("program/meta/bundle.toml")

            assertFalse(result.ok)
            assertTrue(result.message.contains("Invalid UTF-8"))
        } finally {
            root.deleteRecursively()
        }
    }

    @Test
    fun listTomlFiles_separates_alias_chart_and_report_categories() {
        val root = Files.createTempDirectory("config-toml-storage-list").toFile()
        try {
            File(root, "program/config.toml").apply {
                parentFile?.mkdirs()
                writeText("root = true\n")
            }
            File(root, "program/meta/bundle.toml").apply {
                parentFile?.mkdirs()
                writeText("meta = true\n")
            }
            File(root, "user/activity_hierarchy/custom.toml").apply {
                parentFile?.mkdirs()
                writeText("alias = true\n")
            }
            File(root, "user/activity_hierarchy/study.toml").apply {
                parentFile?.mkdirs()
                writeText("legacy_alias = true\n")
            }
            File(root, "user/behavior.toml").apply {
                parentFile?.mkdirs()
                writeText("behavior = true\n")
            }
            File(root, "user/charts.toml").apply {
                parentFile?.mkdirs()
                writeText("charts = true\n")
            }
            File(root, "user/heatmap.toml").apply {
                parentFile?.mkdirs()
                writeText("thresholds = true\n")
            }
            File(root, "program/charts/heatmap.toml").apply {
                parentFile?.mkdirs()
                writeText("chart = true\n")
            }
            File(root, "program/reports/markdown/en/day.toml").apply {
                parentFile?.mkdirs()
                writeText("report = true\n")
            }

            val result = ConfigTomlStorage(root.absolutePath).listTomlFiles()

            assertTrue(result.ok)
            assertEquals(
                listOf(
                    ConfigTomlFileEntry(
                        relativePath = "user/activity_hierarchy/custom.toml",
                        displayName = "user/activity_hierarchy/custom.toml"
                    ),
                    ConfigTomlFileEntry(
                        relativePath = "user/activity_hierarchy/study.toml",
                        displayName = "user/activity_hierarchy/study.toml"
                    ),
                    ConfigTomlFileEntry(
                        relativePath = "user/behavior.toml",
                        displayName = "user/behavior.toml"
                    ),
                    ConfigTomlFileEntry(
                        relativePath = "user/charts.toml",
                        displayName = "user/charts.toml"
                    ),
                    ConfigTomlFileEntry(
                        relativePath = "user/heatmap.toml",
                        displayName = "user/heatmap.toml"
                    )
                ),
                result.userFiles
            )
            assertEquals(
                listOf(
                    ConfigTomlFileEntry(
                        relativePath = "user/activity_hierarchy/custom.toml",
                        displayName = "custom.toml"
                    ),
                    ConfigTomlFileEntry(
                        relativePath = "user/activity_hierarchy/study.toml",
                        displayName = "study.toml"
                    )
                ),
                result.aliasFiles
            )
            assertEquals(
                listOf(
                    ConfigTomlFileEntry(
                        relativePath = "program/charts/heatmap.toml",
                        displayName = "heatmap.toml"
                    )
                ),
                result.chartFiles
            )
            assertEquals(
                listOf(
                    ConfigTomlFileEntry(
                        relativePath = "program/config.toml",
                        displayName = "program/config.toml"
                    ),
                    ConfigTomlFileEntry(
                        relativePath = "program/meta/bundle.toml",
                        displayName = "program/meta/bundle.toml"
                    )
                ),
                result.metaFiles
            )
            assertEquals(
                listOf(
                    ConfigTomlFileEntry(
                        relativePath = "program/reports/markdown/en/day.toml",
                        displayName = "markdown/en/day.toml"
                    )
                ),
                result.reportFiles
            )
        } finally {
            root.deleteRecursively()
        }
    }
}
