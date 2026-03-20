package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Test
import java.io.File
import java.nio.file.Files

class AutoSyncMaterializerTest {
    @Test
    fun materialize_rewritesLegacyTxtAsCanonicalUtf8() {
        val root = Files.createTempDirectory("auto-sync-materializer").toFile()
        try {
            val liveRaw = File(root, "live_raw")
            val liveAutoSync = File(root, "live_auto_sync")
            val rawFile = File(liveRaw, "2026/2026-03.txt")
            rawFile.parentFile?.mkdirs()
            rawFile.writeBytes(
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

            AutoSyncMaterializer().materialize(liveRaw.absolutePath, liveAutoSync.absolutePath)

            val synced = File(liveAutoSync, "2026/2026-03.txt")
            assertEquals("y2026\nm03\n", synced.readText())
        } finally {
            root.deleteRecursively()
        }
    }
}
