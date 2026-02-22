package com.example.tracer

import java.io.OutputStream

private val UTF8_BOM = byteArrayOf(0xEF.toByte(), 0xBB.toByte(), 0xBF.toByte())

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
        val month = parseMonthFromTxtContent(result.content) ?: continue
        if (!index.containsKey(month)) {
            index[month] = result.filePath.replace('\\', '/')
        }
    }
    return index
}

internal fun parseMonthFromTxtContent(content: String): String? {
    var year: String? = null
    var month: String? = null
    val yearRegex = Regex("""^y(\d{4})$""")
    val monthRegex = Regex("""^m(\d{2})$""")

    for (rawLine in content.lineSequence()) {
        val line = rawLine.trim()
        if (line.isEmpty()) {
            continue
        }

        val yearMatch = yearRegex.matchEntire(line)
        if (yearMatch != null && year == null) {
            year = yearMatch.groupValues[1]
            continue
        }

        val monthMatch = monthRegex.matchEntire(line)
        if (monthMatch != null && month == null) {
            if (year == null) {
                return null
            }
            val monthValue = monthMatch.groupValues[1].toIntOrNull() ?: continue
            if (monthValue in 1..12) {
                month = String.format(java.util.Locale.US, "%02d", monthValue)
                continue
            }
        }
    }

    if (year == null || month == null) {
        return null
    }
    return "$year-$month"
}

internal fun normalizeMonthKey(value: String): String? {
    val normalized = value.trim()
    val match = Regex("""^(\d{4})-(\d{2})$""").matchEntire(normalized)
        ?: return null
    val year = match.groupValues[1]
    val monthValue = match.groupValues[2].toIntOrNull() ?: return null
    if (monthValue !in 1..12) {
        return null
    }
    return "$year-${String.format(java.util.Locale.US, "%02d", monthValue)}"
}

internal fun writeUtf8Text(
    output: OutputStream,
    content: String,
    includeBom: Boolean
) {
    val normalizedContent = content.removePrefix("\uFEFF")
    if (includeBom) {
        output.write(UTF8_BOM)
    }
    output.write(normalizedContent.toByteArray(Charsets.UTF_8))
}

internal fun extractExportYearFromFirstLine(content: String): String? {
    val firstLine = content.lineSequence().firstOrNull() ?: return null
    val normalized = firstLine.removePrefix("\uFEFF").trim()
    val match = Regex("""^y(\d{4})$""").matchEntire(normalized) ?: return null
    return match.groupValues[1]
}
