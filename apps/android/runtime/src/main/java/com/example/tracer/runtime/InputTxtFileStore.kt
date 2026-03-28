package com.example.tracer

import java.io.File

internal class InputTxtFileStore {
    fun listTxtFiles(inputRootPath: String): TxtHistoryListResult {
        val root = File(inputRootPath)
        if (!root.exists()) {
            return TxtHistoryListResult(
                ok = true,
                files = emptyList(),
                message = "No TXT input directory."
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

    fun readTxtFile(inputRootPath: String, relativePath: String): TxtFileContentResult {
        val requested = relativePath.trim()
        if (requested.isEmpty()) {
            return TxtFileContentResult(
                ok = false,
                filePath = "",
                content = "",
                message = "TXT file path is empty."
            )
        }

        val root = File(inputRootPath).canonicalFile
        val target = File(root, requested).canonicalFile
        val relative = target.relativeToOrNull(root)
        if (relative == null) {
            return TxtFileContentResult(
                ok = false,
                filePath = requested,
                content = "",
                message = "TXT path is outside input root."
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

        val canonicalContent = runCatching {
            CanonicalTextCodec.readFile(target)
        }.getOrElse { error ->
            return TxtFileContentResult(
                ok = false,
                filePath = requested,
                content = "",
                message = "Cannot read TXT file: ${error.message ?: "unknown read error"}"
            )
        }

        return TxtFileContentResult(
            ok = true,
            filePath = relative.invariantSeparatorsPath,
            content = canonicalContent,
            message = "Read TXT success."
        )
    }

    fun writeTxtFile(inputRootPath: String, relativePath: String, content: String): TxtFileContentResult {
        val requested = relativePath.trim()
        if (requested.isEmpty()) {
            return TxtFileContentResult(
                ok = false,
                filePath = "",
                content = "",
                message = "TXT file path is empty."
            )
        }

        val root = File(inputRootPath).canonicalFile
        val target = File(root, requested).canonicalFile
        val relative = target.relativeToOrNull(root)
        if (relative == null) {
            return TxtFileContentResult(
                ok = false,
                filePath = requested,
                content = "",
                message = "TXT path is outside input root."
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

        val canonicalContent = CanonicalTextCodec.canonicalizeText(content)
        runCatching {
            CanonicalTextCodec.writeFile(target, canonicalContent)
        }.getOrElse { error ->
            return TxtFileContentResult(
                ok = false,
                filePath = requested,
                content = "",
                message = "Cannot write TXT file: ${error.message ?: "unknown write error"}"
            )
        }
        return TxtFileContentResult(
            ok = true,
            filePath = relative.invariantSeparatorsPath,
            content = canonicalContent,
            message = "Save TXT success."
        )
    }
}
