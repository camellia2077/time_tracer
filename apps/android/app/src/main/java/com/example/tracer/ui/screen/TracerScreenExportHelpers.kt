package com.example.tracer

internal suspend fun buildMonthToSourcePathIndex(
    historyFiles: List<String>,
    readTxtFile: suspend (String) -> TxtFileContentResult
): Map<String, String> {
    val index = linkedMapOf<String, String>()
    for (path in historyFiles) {
        val result = readTxtFile(path)
        if (!result.ok) {
            continue
        }
        val month = parseTxtMonthKey(result.content) ?: continue
        if (!index.containsKey(month)) {
            index[month] = result.filePath.replace('\\', '/')
        }
    }
    return index
}

internal fun parseMonthFromTxtContent(content: String): String? {
    return parseTxtMonthKey(content)
}

internal fun normalizeMonthKey(value: String): String? {
    return normalizeTxtMonthKey(value)
}

internal fun extractExportYearFromFirstLine(content: String): String? {
    return parseTxtMonthHeader(content)?.year?.toString()
}
