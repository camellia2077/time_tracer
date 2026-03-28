package com.example.tracer

internal interface TextStorage {
    fun listTxtFiles(): TxtHistoryListResult
    fun readTxtFile(relativePath: String): TxtFileContentResult
    fun writeTxtFile(relativePath: String, content: String): TxtFileContentResult
}

internal class CanonicalInputTextStorage(
    private val inputRootPath: String,
    private val recordStore: InputRecordStore
) : TextStorage {
    override fun listTxtFiles(): TxtHistoryListResult {
        return recordStore.listTxtFiles(inputRootPath)
    }

    override fun readTxtFile(relativePath: String): TxtFileContentResult {
        return recordStore.readTxtFile(inputRootPath, relativePath)
    }

    override fun writeTxtFile(relativePath: String, content: String): TxtFileContentResult {
        return recordStore.writeTxtFile(
            inputRootPath = inputRootPath,
            relativePath = relativePath,
            content = content
        )
    }
}

internal fun resolveCanonicalTxtRelativePath(inputRootPath: String, relativePath: String): String? {
    val requested = relativePath.trim().replace('\\', '/')
    if (requested.isEmpty()) {
        return null
    }
    val root = java.io.File(inputRootPath).canonicalFile
    val target = java.io.File(root, requested).canonicalFile
    val relative = target.relativeToOrNull(root) ?: return null
    if (!target.extension.equals("txt", ignoreCase = true)) {
        return null
    }
    return relative.invariantSeparatorsPath
}
