package com.example.tracer

enum class FileCryptoSecurityLevel(val wireValue: String) {
    INTERACTIVE("interactive"),
    MODERATE("moderate"),
    HIGH("high");

    companion object {
        fun fromWireValue(value: String): FileCryptoSecurityLevel = when (value.lowercase()) {
            "moderate" -> MODERATE
            "high" -> HIGH
            else -> INTERACTIVE
        }
    }
}
