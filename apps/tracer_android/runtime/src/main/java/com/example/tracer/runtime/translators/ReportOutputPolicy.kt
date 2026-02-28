package com.example.tracer

internal object ReportOutputPolicy {
    const val TEXT_ENCODING = "UTF-8"
    const val NEWLINE = "\n"
    const val PRESERVE_TRAILING_NEWLINE = true

    fun preserveRaw(content: String): String = content
}

