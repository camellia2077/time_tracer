package com.example.tracer

internal fun filterDigits(value: String, maxLength: Int): String =
    value.filter { it.isDigit() }.take(maxLength)

internal fun splitDateDigits(value: String): Triple<String, String, String> {
    val digits = filterDigits(value, 8)
    return Triple(
        digits.take(4),
        digits.drop(4).take(2),
        digits.drop(6).take(2)
    )
}

internal fun mergeDateDigits(year: String, month: String, day: String): String =
    filterDigits(year, 4) + filterDigits(month, 2) + filterDigits(day, 2)

internal fun splitYearMonthDigits(value: String): Pair<String, String> {
    val digits = filterDigits(value, 6)
    return Pair(digits.take(4), digits.drop(4).take(2))
}

internal fun mergeYearMonthDigits(year: String, month: String): String =
    filterDigits(year, 4) + filterDigits(month, 2)

internal fun splitYearWeekDigits(value: String): Pair<String, String> {
    val digits = filterDigits(value, 6)
    return Pair(digits.take(4), digits.drop(4).take(2))
}

internal fun mergeYearWeekDigits(year: String, week: String): String =
    filterDigits(year, 4) + filterDigits(week, 2)
