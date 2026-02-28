package com.example.tracer

import java.io.File

internal class LiveRawTxtFileStore {
    fun listTxtFiles(liveRawInputPath: String): TxtHistoryListResult {
        val root = File(liveRawInputPath)
        if (!root.exists()) {
            return TxtHistoryListResult(
                ok = true,
                files = emptyList(),
                message = "No live TXT directory."
            )
        }

        val files = root.walkTopDown()
            .filter { it.isFile && it.extension.equals("txt", ignoreCase = true) }
            .map { it.relativeTo(root).invariantSeparatorsPath }
            .sorted()
            .toList()

        return TxtHistoryListResult(
            ok = true,
            files = files,
            message = "Found ${files.size} TXT file(s)."
        )
    }

    fun readTxtFile(liveRawInputPath: String, relativePath: String): TxtFileContentResult {
        val requested = relativePath.trim()
        if (requested.isEmpty()) {
            return TxtFileContentResult(
                ok = false,
                filePath = "",
                content = "",
                message = "TXT file path is empty."
            )
        }

        val root = File(liveRawInputPath).canonicalFile
        val target = File(root, requested).canonicalFile
        val relative = target.relativeToOrNull(root)
        if (relative == null) {
            return TxtFileContentResult(
                ok = false,
                filePath = requested,
                content = "",
                message = "TXT path is outside live input root."
            )
        }
        if (!target.exists() || !target.isFile) {
            return TxtFileContentResult(
                ok = false,
                filePath = requested,
                content = "",
                message = "TXT file not found."
            )
        }

        return TxtFileContentResult(
            ok = true,
            filePath = relative.invariantSeparatorsPath,
            content = target.readText(),
            message = "Read TXT success."
        )
    }

    fun writeTxtFile(liveRawInputPath: String, relativePath: String, content: String): TxtFileContentResult {
        val requested = relativePath.trim()
        if (requested.isEmpty()) {
            return TxtFileContentResult(
                ok = false,
                filePath = "",
                content = "",
                message = "TXT file path is empty."
            )
        }

        val root = File(liveRawInputPath).canonicalFile
        val target = File(root, requested).canonicalFile
        val relative = target.relativeToOrNull(root)
        if (relative == null) {
            return TxtFileContentResult(
                ok = false,
                filePath = requested,
                content = "",
                message = "TXT path is outside live input root."
            )
        }
        if (!target.exists() || !target.isFile) {
            return TxtFileContentResult(
                ok = false,
                filePath = requested,
                content = "",
                message = "TXT file not found."
            )
        }

        target.writeText(content)
        return TxtFileContentResult(
            ok = true,
            filePath = relative.invariantSeparatorsPath,
            content = content,
            message = "Save TXT success."
        )
    }
}
