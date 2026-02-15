package com.example.tracer

import java.io.File

internal class AutoSyncMaterializer {
    fun materialize(liveRawInputPath: String, liveAutoSyncInputPath: String) {
        val rawRoot = File(liveRawInputPath)
        val syncRoot = File(liveAutoSyncInputPath)

        if (syncRoot.exists()) {
            syncRoot.deleteRecursively()
        }
        syncRoot.mkdirs()

        if (!rawRoot.exists()) {
            return
        }

        val rawFiles = rawRoot.walkTopDown()
            .filter { it.isFile && it.extension.equals("txt", ignoreCase = true) }
            .toList()

        for (rawFile in rawFiles) {
            val target = File(syncRoot, rawFile.relativeTo(rawRoot).invariantSeparatorsPath)
            target.parentFile?.mkdirs()
            target.writeText(rawFile.readText())
        }
    }
}
