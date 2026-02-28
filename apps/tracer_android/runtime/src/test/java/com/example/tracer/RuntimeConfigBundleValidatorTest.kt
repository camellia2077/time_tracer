package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test
import java.io.File
import java.nio.file.Files

class RuntimeConfigBundleValidatorTest {
    @Test
    fun validateRuntimeConfigBundle_validBundle_returnsOk() {
        val configRoot = Files.createTempDirectory("runtime-config-valid").toFile()
        try {
            writeBundleToml(configRoot, profile = "android")
            writeRequiredFiles(configRoot)

            val result = validateRuntimeConfigBundle(configRoot)

            assertTrue(result.ok)
            assertEquals(1, result.schemaVersion)
            assertEquals("android", result.profile)
            assertTrue(result.missingFiles.isEmpty())
        } finally {
            configRoot.deleteRecursively()
        }
    }

    @Test
    fun validateRuntimeConfigBundle_missingRequiredFile_returnsFailure() {
        val configRoot = Files.createTempDirectory("runtime-config-missing").toFile()
        try {
            writeBundleToml(configRoot, profile = "android")
            writeRequiredFiles(configRoot)
            File(configRoot, "reports/markdown/year.toml").delete()

            val result = validateRuntimeConfigBundle(configRoot)

            assertFalse(result.ok)
            assertTrue(result.missingFiles.contains("reports/markdown/year.toml"))
        } finally {
            configRoot.deleteRecursively()
        }
    }

    @Test
    fun validateRuntimeConfigBundle_profileMismatch_returnsFailure() {
        val configRoot = Files.createTempDirectory("runtime-config-profile").toFile()
        try {
            writeBundleToml(configRoot, profile = "desktop")
            writeRequiredFiles(configRoot)

            val result = validateRuntimeConfigBundle(configRoot)

            assertFalse(result.ok)
            assertEquals("desktop", result.profile)
            assertTrue(result.message.contains("profile mismatch"))
        } finally {
            configRoot.deleteRecursively()
        }
    }

    private fun writeBundleToml(configRoot: File, profile: String) {
        val bundleFile = File(configRoot, "meta/bundle.toml")
        bundleFile.parentFile?.mkdirs()
        bundleFile.writeText(
            """
            schema_version = 1
            profile = "$profile"
            bundle_name = "tracer_core_config"
            
            [file_list]
            required = [
              "config.toml",
              "converter/interval_processor_config.toml",
              "converter/alias_mapping.toml",
              "converter/duration_rules.toml",
              "reports/markdown/day.toml",
              "reports/markdown/month.toml",
              "reports/markdown/period.toml",
              "reports/markdown/week.toml",
              "reports/markdown/year.toml",
            ]
            """.trimIndent()
        )
    }

    private fun writeRequiredFiles(configRoot: File) {
        val requiredFiles = listOf(
            "config.toml",
            "converter/interval_processor_config.toml",
            "converter/alias_mapping.toml",
            "converter/duration_rules.toml",
            "reports/markdown/day.toml",
            "reports/markdown/month.toml",
            "reports/markdown/period.toml",
            "reports/markdown/week.toml",
            "reports/markdown/year.toml"
        )
        for (relativePath in requiredFiles) {
            val file = File(configRoot, relativePath)
            file.parentFile?.mkdirs()
            file.writeText("dummy=true")
        }
    }
}
