package com.example.tracer

private val TxtMonthKeyPattern = Regex("""^(\d{4})-(\d{2})$""")

internal fun deriveTxtYearOptions(availableMonths: List<String>): List<String> {
    return availableMonths
        .mapNotNull { monthKey ->
            val match = TxtMonthKeyPattern.matchEntire(monthKey.trim()) ?: return@mapNotNull null
            val month = match.groupValues[2].toIntOrNull() ?: return@mapNotNull null
            if (month !in 1..12) {
                return@mapNotNull null
            }
            match.groupValues[1]
        }
        .distinct()
        .sortedDescending()
}
