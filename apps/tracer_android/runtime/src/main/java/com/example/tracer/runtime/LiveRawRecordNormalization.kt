package com.example.tracer

import java.util.Locale

internal class LiveRawRecordNormalization {
    fun normalizeActivityName(raw: String): String {
        val cleaned = raw.replace("\n", " ").replace("\r", " ").trim()
        if (cleaned.isEmpty()) {
            return ""
        }
        return when (cleaned.lowercase(Locale.US)) {
            "zh" -> "zhihu"
            else -> cleaned
        }
    }

    fun normalizeRemark(raw: String): String {
        return raw.replace("\n", " ").replace("\r", " ").trim()
    }

    fun normalizeForComparison(value: String): String {
        return value.trim()
            .lowercase(Locale.US)
            .replace("\\s+".toRegex(), "")
    }
}
