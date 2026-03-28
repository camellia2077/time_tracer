package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test
import java.io.File
import java.nio.file.Files

class InputTxtFileStoreTest {
    @Test
    fun readTxtFile_canonicalizesLegacyUtf8Text() {
        val root = Files.createTempDirectory("live-raw-txt-store").toFile()
        try {
            val target = File(root, "2026/2026-03.txt")
            target.parentFile?.mkdirs()
            target.writeBytes(
                byteArrayOf(
                    0xEF.toByte(),
                    0xBB.toByte(),
                    0xBF.toByte(),
                    'y'.code.toByte(),
                    '2'.code.toByte(),
                    '0'.code.toByte(),
                    '2'.code.toByte(),
                    '6'.code.toByte(),
                    '\r'.code.toByte(),
                    '\n'.code.toByte(),
                    'm'.code.toByte(),
                    '0'.code.toByte(),
                    '3'.code.toByte(),
                    '\r'.code.toByte()
                )
            )

            val result = InputTxtFileStore().readTxtFile(root.absolutePath, "2026/2026-03.txt")

            assertTrue(result.ok)
            assertEquals("y2026\nm03\n", result.content)
        } finally {
            root.deleteRecursively()
        }
    }

    @Test
    fun readTxtFile_invalidUtf8ReturnsFailure() {
        val root = Files.createTempDirectory("live-raw-txt-store-invalid").toFile()
        try {
            val target = File(root, "broken.txt")
            target.writeBytes(byteArrayOf(0xFF.toByte()))

            val result = InputTxtFileStore().readTxtFile(root.absolutePath, "broken.txt")

            assertFalse(result.ok)
            assertTrue(result.message.contains("Invalid UTF-8"))
        } finally {
            root.deleteRecursively()
        }
    }

    @Test
    fun writeTxtFile_rewritesContentAsCanonicalUtf8() {
        val root = Files.createTempDirectory("live-raw-txt-store-write").toFile()
        try {
            val target = File(root, "2026/2026-03.txt")
            target.parentFile?.mkdirs()
            target.writeText("seed")

            val result = InputTxtFileStore().writeTxtFile(
                inputRootPath = root.absolutePath,
                relativePath = "2026/2026-03.txt",
                content = "\uFEFFy2026\r\nm03\r"
            )

            assertTrue(result.ok)
            assertEquals("y2026\nm03\n", target.readText())
        } finally {
            root.deleteRecursively()
        }
    }
}
