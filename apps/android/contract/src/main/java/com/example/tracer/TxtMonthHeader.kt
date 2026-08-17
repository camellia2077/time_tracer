package com.example.tracer

import java.util.Locale

private const val FIRST_MONTH = 1
private const val LAST_MONTH = 12
private const val NEXT_LINE_OFFSET = 1
private const val YEAR_DIGITS = 4
private const val MONTH_DIGITS = 2

data class TxtMonthHeader(
    val year: Int,
    val month: Int
) {
    val monthKey: String
        get() = String.format(
            Locale.US,
            "%0${YEAR_DIGITS}d-%0${MONTH_DIGITS}d",
            year,
            month
        )

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
    val yearRegex = Regex("""^y(\d{4})$""")
    val monthRegex = Regex("""^m(\d{2})$""")
    val lines = canonicalizeTxtHeaderContent(content)
        .lineSequence()
        .map(String::trim)
        .filter(String::isNotEmpty)
        .toList()
    val yearIndex = lines.indexOfFirst { yearRegex.matches(it) }
    if (yearIndex < 0) {
        return null
    }
    val yearValue = yearRegex.matchEntire(lines[yearIndex])
        ?.groupValues
        ?.getOrNull(1)
        ?.toIntOrNull()
        ?: return null
    val monthValue = lines
        .drop(yearIndex + NEXT_LINE_OFFSET)
        .asSequence()
        .mapNotNull { line ->
            monthRegex.matchEntire(line)?.groupValues?.getOrNull(1)?.toIntOrNull()
        }
        .firstOrNull()
        ?: return null
    if (monthValue !in FIRST_MONTH..LAST_MONTH) {
        return null
    }
    return TxtMonthHeader(year = yearValue, month = monthValue)
}

fun parseTxtMonthKey(content: String): String? = parseTxtMonthHeader(content)?.monthKey

fun normalizeTxtMonthKey(value: String): String? {
    val match = Regex("""^(\d{4})-(\d{2})$""").matchEntire(value.trim()) ?: return null
    val year = match.groupValues[1].toIntOrNull() ?: return null
    val month = match.groupValues[2].toIntOrNull() ?: return null
    if (month !in FIRST_MONTH..LAST_MONTH) {
        return null
    }
    return TxtMonthHeader(year = year, month = month).monthKey
}

fun buildCanonicalTxtRelativePath(monthKey: String): String? {
    val normalized = normalizeTxtMonthKey(monthKey) ?: return null
    return "${normalized.substring(0, YEAR_DIGITS)}/$normalized.txt"
}
