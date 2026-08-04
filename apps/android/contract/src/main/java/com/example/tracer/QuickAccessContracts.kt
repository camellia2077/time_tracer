package com.example.tracer

data class QuickAccessResult(
    val ok: Boolean,
    val aliases: List<String> = emptyList(),
    val message: String = ""
)
