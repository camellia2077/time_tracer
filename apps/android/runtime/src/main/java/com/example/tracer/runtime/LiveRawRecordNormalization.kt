package com.example.tracer

import java.util.Locale

internal class LiveRawRecordNormalization {
    fun normalizeActivityName(raw: String): String {
        // Keep raw alias key text: record-time should only sanitize user input,
        // not convert alias key -> canonical value.
        val cleaned = raw.replace("\n", " ").replace("\r", " ").trim()
        if (cleaned.isEmpty()) {
            return ""
        }
        return cleaned
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
