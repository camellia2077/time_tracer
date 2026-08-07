package com.example.tracer

enum class TracerExchangeSecurityLevel(val wireValue: String) {
    INTERACTIVE("interactive"),
    MODERATE("moderate"),
    HIGH("high");

    companion object {
        fun fromWireValue(value: String): TracerExchangeSecurityLevel = when (value.lowercase()) {
            "moderate" -> MODERATE
            "high" -> HIGH
            else -> INTERACTIVE
        }
    }
}
