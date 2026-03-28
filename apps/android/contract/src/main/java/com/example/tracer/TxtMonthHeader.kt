package com.example.tracer

import java.util.Locale

data class TxtMonthHeader(
    val year: Int,
    val month: Int
) {
    val monthKey: String
        get() = String.format(Locale.US, "%04d-%02d", year, month)

    val canonicalRelativePath: String
        get() = "$year/$monthKey.txt"
}

fun canonicalizeTxtHeaderContent(content: String): String {
    return content
        .removePrefix("\uFEFF")
        .replace("\r\n", "\n")
        .replace('\r', '\n')
}

fun parseTxtMonthHeader(content: String): TxtMonthHeader? {
    var yearValue: Int? = null
    var monthValue: Int? = null
    val yearRegex = Regex("""^y(\d{4})$""")
    val monthRegex = Regex("""^m(\d{2})$""")

    for (rawLine in canonicalizeTxtHeaderContent(content).lineSequence()) {
        val line = rawLine.trim()
        if (line.isEmpty()) {
            continue
        }

        val yearMatch = yearRegex.matchEntire(line)
        if (yearMatch != null && yearValue == null) {
            yearValue = yearMatch.groupValues[1].toIntOrNull()
            continue
        }

        val monthMatch = monthRegex.matchEntire(line)
        if (monthMatch != null && yearValue != null && monthValue == null) {
            val parsedMonth = monthMatch.groupValues[1].toIntOrNull() ?: return null
            if (parsedMonth !in 1..12) {
                return null
            }
            monthValue = parsedMonth
            break
        }
    }

    if (yearValue == null || monthValue == null) {
        return null
    }
    return TxtMonthHeader(year = yearValue, month = monthValue)
}

fun parseTxtMonthKey(content: String): String? = parseTxtMonthHeader(content)?.monthKey

fun normalizeTxtMonthKey(value: String): String? {
    val match = Regex("""^(\d{4})-(\d{2})$""").matchEntire(value.trim()) ?: return null
    val year = match.groupValues[1].toIntOrNull() ?: return null
    val month = match.groupValues[2].toIntOrNull() ?: return null
    if (month !in 1..12) {
        return null
    }
    return TxtMonthHeader(year = year, month = month).monthKey
}

fun buildCanonicalTxtRelativePath(monthKey: String): String? {
    val normalized = normalizeTxtMonthKey(monthKey) ?: return null
    return "${normalized.substring(0, 4)}/$normalized.txt"
}
