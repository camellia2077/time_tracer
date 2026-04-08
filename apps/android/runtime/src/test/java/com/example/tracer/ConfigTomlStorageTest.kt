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
            val target = File(root, "meta/bundle.toml")
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

            val result = ConfigTomlStorage(root.absolutePath).readTomlFile("meta/bundle.toml")

            assertTrue(result.ok)
            assertEquals("schema_version = 1\n", result.content)
        } finally {
            root.deleteRecursively()
        }
    }

    @Test
    fun writeTomlFile_rewritesContentAsCanonicalUtf8() {
        val root = Files.createTempDirectory("config-toml-storage-write").toFile()
        try {
            val target = File(root, "meta/bundle.toml")
            target.parentFile?.mkdirs()
            target.writeText("seed")

            val result = ConfigTomlStorage(root.absolutePath).writeTomlFile(
                relativePath = "meta/bundle.toml",
                content = "\uFEFFprofile = \"android\"\r\n"
            )

            assertTrue(result.ok)
            assertEquals("profile = \"android\"\n", target.readText())
        } finally {
            root.deleteRecursively()
        }
    }

    @Test
    fun readTomlFile_invalidUtf8ReturnsFailure() {
        val root = Files.createTempDirectory("config-toml-storage-invalid").toFile()
        try {
            val target = File(root, "meta/bundle.toml")
            target.parentFile?.mkdirs()
            target.writeBytes(byteArrayOf(0xFF.toByte()))

            val result = ConfigTomlStorage(root.absolutePath).readTomlFile("meta/bundle.toml")

            assertFalse(result.ok)
            assertTrue(result.message.contains("Invalid UTF-8"))
        } finally {
            root.deleteRecursively()
        }
    }

    @Test
    fun listTomlFiles_separates_converter_chart_and_report_categories() {
        val root = Files.createTempDirectory("config-toml-storage-list").toFile()
        try {
            File(root, "config.toml").writeText("root = true\n")
            File(root, "meta/bundle.toml").apply {
                parentFile?.mkdirs()
                writeText("meta = true\n")
            }
            File(root, "converter/interval_processor_config.toml").apply {
                parentFile?.mkdirs()
                writeText("converter = true\n")
            }
            File(root, "charts/heatmap.toml").apply {
                parentFile?.mkdirs()
                writeText("chart = true\n")
            }
            File(root, "reports/markdown/day.toml").apply {
                parentFile?.mkdirs()
                writeText("report = true\n")
            }

            val result = ConfigTomlStorage(root.absolutePath).listTomlFiles()

            assertTrue(result.ok)
            assertEquals(
                listOf(
                    ConfigTomlFileEntry(
                        relativePath = "converter/interval_processor_config.toml",
                        displayName = "interval_processor_config.toml"
                    )
                ),
                result.converterFiles
            )
            assertEquals(
                listOf(
                    ConfigTomlFileEntry(
                        relativePath = "charts/heatmap.toml",
                        displayName = "heatmap.toml"
                    )
                ),
                result.chartFiles
            )
            assertEquals(
                listOf(
                    ConfigTomlFileEntry(
                        relativePath = "config.toml",
                        displayName = "config.toml"
                    ),
                    ConfigTomlFileEntry(
                        relativePath = "meta/bundle.toml",
                        displayName = "meta/bundle.toml"
                    )
                ),
                result.metaFiles
            )
            assertEquals(
                listOf(
                    ConfigTomlFileEntry(
                        relativePath = "reports/markdown/day.toml",
                        displayName = "markdown/day.toml"
                    )
                ),
                result.reportFiles
            )
        } finally {
            root.deleteRecursively()
        }
    }
}
